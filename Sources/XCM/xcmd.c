#define _GNU_SOURCE
#include "include/xcm/protocol.h"

#include <errno.h>
#include <grp.h>
#include <poll.h>
#include <pwd.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/un.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>
#include <stddef.h>
#include <sys/utsname.h>

#define XCM_SOCKET_DEFAULT "/run/mintosh/xcm.sock"
#define XCM_REPLAY_SLOTS 512U
#define XCM_AUDIT_TAG "xcmd"

struct replay_entry {
    uid_t uid;
    uint64_t request_id;
};

static volatile sig_atomic_t stopping;
static struct replay_entry replay_cache[XCM_REPLAY_SLOTS];
static size_t replay_cursor;

static void stop_server(int signal_number) {
    (void)signal_number;
    stopping = 1;
}

static void audit_event(int priority, const char *format, ...) {
    va_list arguments;
    va_start(arguments, format);
    vsyslog(priority, format, arguments);
    va_end(arguments);
}

static bool group_member(uid_t uid, gid_t required_group) {
    struct passwd *account = getpwuid(uid);
    if (account == NULL) return false;
    int count = 16;
    gid_t *groups = calloc((size_t)count, sizeof(*groups));
    if (groups == NULL) return false;
    int result = getgrouplist(account->pw_name, account->pw_gid, groups, &count);
    if (result < 0) {
        gid_t *expanded = realloc(groups, (size_t)count * sizeof(*groups));
        if (expanded == NULL) { free(groups); return false; }
        groups = expanded;
        result = getgrouplist(account->pw_name, account->pw_gid, groups, &count);
    }
    bool found = false;
    if (result >= 0) {
        for (int i = 0; i < count; ++i)
            if (groups[i] == required_group) { found = true; break; }
    }
    free(groups);
    return found;
}

static bool replay_seen(uid_t uid, uint64_t request_id) {
    for (size_t i = 0; i < XCM_REPLAY_SLOTS; ++i)
        if (replay_cache[i].uid == uid &&
            replay_cache[i].request_id == request_id)
            return true;
    replay_cache[replay_cursor].uid = uid;
    replay_cache[replay_cursor].request_id = request_id;
    replay_cursor = (replay_cursor + 1U) % XCM_REPLAY_SLOTS;
    return false;
}

static void send_response(int client, uint64_t request_id,
                          enum xcm_response_status status,
                          uint32_t payload_type, const void *payload,
                          uint32_t payload_size) {
    unsigned char packet[sizeof(struct xcm_response) + XCM_MAX_MESSAGE_SIZE];
    struct xcm_response response = {
        (uint32_t)(sizeof(response) + payload_size),
        XCM_PROTOCOL_VERSION,
        (uint16_t)status,
        request_id,
        payload_type,
        payload_size
    };
    if (payload_size > XCM_MAX_MESSAGE_SIZE ||
        sizeof(response) + payload_size > sizeof(packet))
        return;
    memcpy(packet, &response, sizeof(response));
    if (payload_size != 0 && payload != NULL)
        memcpy(packet + sizeof(response), payload, payload_size);
    (void)send(client, packet, sizeof(response) + payload_size, MSG_NOSIGNAL);
}

static void handle_client(int client, gid_t service_gid) {
    struct ucred credentials;
    socklen_t credentials_size = sizeof(credentials);
    if (getsockopt(client, SOL_SOCKET, SO_PEERCRED,
                   &credentials, &credentials_size) < 0 ||
        credentials_size != sizeof(credentials)) {
        audit_event(LOG_WARNING, "peer credential lookup failed");
        send_response(client, 0, XCM_RESPONSE_DENIED, XCM_RESPONSE_PAYLOAD_NONE, NULL, 0);
        return;
    }

    if (credentials.uid == 0 ||
        !group_member(credentials.uid, service_gid)) {
        audit_event(LOG_WARNING, "denied peer uid=%lu pid=%ld: not an authorized Choca session",
                    (unsigned long)credentials.uid, (long)credentials.pid);
        send_response(client, 0, XCM_RESPONSE_DENIED, XCM_RESPONSE_PAYLOAD_NONE, NULL, 0);
        return;
    }

    unsigned char packet[XCM_MAX_MESSAGE_SIZE];
    ssize_t received = recv(client, packet, sizeof(packet), MSG_TRUNC);
    if (received < (ssize_t)sizeof(struct xcm_request) ||
        received > (ssize_t)sizeof(packet)) {
        audit_event(LOG_WARNING, "invalid message size from uid=%lu",
                    (unsigned long)credentials.uid);
        send_response(client, 0, XCM_RESPONSE_INVALID, XCM_RESPONSE_PAYLOAD_NONE, NULL, 0);
        return;
    }

    struct xcm_request request;
    memcpy(&request, packet, sizeof(request));
    if (request.size != (uint32_t)received ||
        request.payload_size != (uint32_t)received - sizeof(request)) {
        send_response(client, request.request_id, XCM_RESPONSE_INVALID,
                      XCM_RESPONSE_PAYLOAD_NONE, NULL, 0);
        return;
    }

    if (replay_seen(credentials.uid, credentials.pid, request.request_id)) {
        audit_event(LOG_WARNING, "replayed request uid=%lu request=%llu",
                    (unsigned long)credentials.uid,
                    (unsigned long long)request.request_id);
        send_response(client, request.request_id, XCM_RESPONSE_DENIED,
                      XCM_RESPONSE_PAYLOAD_NONE, NULL, 0);
        return;
    }

    struct xcm_policy_context context = {
        .authenticated_peer = XCM_PEER_CHOCA,
        .granted_capabilities = XCM_CAP_PRESENT | XCM_CAP_READ_SYSTEM,
        .maximum_payload_size = 4096,
        .transport_authenticated = true,
        .verified_update_bundle = false,
        .confirmation_grant_valid = false,
        .confirmation_object_id = 0,
        .confirmation_operation = 0
    };
    enum xcm_policy_result decision = xcm_policy_authorize(&request, &context);
    if (decision != XCM_POLICY_ALLOW) {
        audit_event(LOG_WARNING,
                    "policy denied uid=%lu pid=%ld request=%llu operation=%u target=%u reason=%u",
                    (unsigned long)credentials.uid, (long)credentials.pid,
                    (unsigned long long)request.request_id, request.operation,
                    request.target, (unsigned)decision);
        send_response(client, request.request_id, XCM_RESPONSE_DENIED,
                      XCM_RESPONSE_PAYLOAD_NONE, NULL, 0);
        return;
    }

    if (request.operation == XCM_OP_READ_SYSTEM_INFO) {
        struct utsname system;
        char info[512];
        if (uname(&system) < 0) {
            send_response(client, request.request_id, XCM_RESPONSE_INTERNAL_ERROR,
                          XCM_RESPONSE_PAYLOAD_NONE, NULL, 0);
            return;
        }
        int length = snprintf(info, sizeof(info),
                              "Mintosh; kernel=%s; release=%s; machine=%s",
                              system.sysname, system.release, system.machine);
        if (length < 0 || (size_t)length >= sizeof(info)) {
            send_response(client, request.request_id, XCM_RESPONSE_INTERNAL_ERROR,
                          XCM_RESPONSE_PAYLOAD_NONE, NULL, 0);
            return;
        }
        audit_event(LOG_INFO, "allowed system-info query uid=%lu request=%llu",
                    (unsigned long)credentials.uid,
                    (unsigned long long)request.request_id);
        send_response(client, request.request_id, XCM_RESPONSE_OK,
                      XCM_RESPONSE_DARKIN_DATA, info, (uint32_t)length);
        return;
    }

    /* The renderer is a separate Choca endpoint and is not yet registered. */
    send_response(client, request.request_id, XCM_RESPONSE_UNAVAILABLE,
                  XCM_RESPONSE_PAYLOAD_NONE, NULL, 0);
}

static int prepare_socket(const char *path, gid_t group_id) {
    char directory[sizeof(((struct sockaddr_un *)0)->sun_path)];
    size_t length = strlen(path);
    if (length == 0 || length >= sizeof(directory)) { errno = ENAMETOOLONG; return -1; }
    memcpy(directory, path, length + 1U);
    char *slash = strrchr(directory, '/');
    if (slash == NULL || slash == directory) { errno = EINVAL; return -1; }
    *slash = '\0';

    struct stat metadata;
    if (lstat(directory, &metadata) < 0 ||
        !S_ISDIR(metadata.st_mode) ||
        metadata.st_uid != 0 ||
        (metadata.st_mode & 0022) != 0) {
        errno = EPERM;
        return -1;
    }

    if (lstat(path, &metadata) == 0) {
        if (!S_ISSOCK(metadata.st_mode) || metadata.st_uid != 0) {
            errno = EPERM;
            return -1;
        }
        if (unlink(path) < 0) return -1;
    } else if (errno != ENOENT) {
        return -1;
    }

    int server = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0);
    if (server < 0) return -1;
    struct sockaddr_un address;
    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    memcpy(address.sun_path, path, length + 1U);
    if (bind(server, (struct sockaddr *)&address, sizeof(address)) < 0 ||
        chown(path, 0, group_id) < 0 ||
        chmod(path, 0660) < 0 ||
        listen(server, 32) < 0) {
        int saved = errno;
        close(server);
        unlink(path);
        errno = saved;
        return -1;
    }
    return server;
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        puts("XCM Broker 0.1.0");
        return 0;
    }
    if (argc != 1) {
        fputs("Usage: xcmd [--version]\n", stderr);
        return 64;
    }
    if (geteuid() != 0) {
        fputs("xcmd must run as root; Choca clients remain unprivileged.\n", stderr);
        return 77;
    }

    struct group *services = getgrnam("mintosh-services");
    if (services == NULL) {
        fputs("xcmd: required mintosh-services group is missing\n", stderr);
        return 1;
    }
    const char *socket_path = getenv("MINTOSH_XCM_SOCKET");
    if (socket_path == NULL || socket_path[0] == '\0')
        socket_path = XCM_SOCKET_DEFAULT;

    int server = prepare_socket(socket_path, services->gr_gid);
    if (server < 0) {
        perror("xcmd: prepare socket");
        return 1;
    }

    openlog(XCM_AUDIT_TAG, LOG_PID | LOG_NDELAY, LOG_AUTHPRIV);
    signal(SIGTERM, stop_server);
    signal(SIGINT, stop_server);
    signal(SIGPIPE, SIG_IGN);
    audit_event(LOG_NOTICE, "XCM broker started");

    while (!stopping) {
        struct pollfd descriptor = { .fd = server, .events = POLLIN };
        int ready = poll(&descriptor, 1, 1000);
        if (ready < 0) {
            if (errno == EINTR) continue;
            audit_event(LOG_ERR, "poll failed: %s", strerror(errno));
            break;
        }
        if (ready == 0) continue;
        int client = accept4(server, NULL, NULL, SOCK_CLOEXEC);
        if (client < 0) {
            if (errno == EINTR || errno == EAGAIN) continue;
            audit_event(LOG_ERR, "accept failed: %s", strerror(errno));
            break;
        }
        struct timeval timeout = { .tv_sec = 2, .tv_usec = 0 };
        (void)setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        handle_client(client, services->gr_gid);
        close(client);
    }

    close(server);
    unlink(socket_path);
    audit_event(LOG_NOTICE, "XCM broker stopped");
    closelog();
    return 0;
}

#define _GNU_SOURCE
#include "include/sean/protocol.h"

#include <errno.h>
#include <grp.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/un.h>
#include <syslog.h>
#include <sys/utsname.h>
#include <unistd.h>

static volatile sig_atomic_t stopping;

static void stop_server(int signal_number)
{
    (void)signal_number;
    stopping = 1;
}

static void reply(int client, uint64_t request_id, enum sean_status status,
                  const char *payload, uint32_t payload_size)
{
    unsigned char packet[sizeof(struct sean_response) + SEAN_MAX_PACKET_SIZE];
    struct sean_response response = {
        (uint32_t)(sizeof(response) + payload_size),
        SEAN_PROTOCOL_VERSION,
        (uint16_t)status,
        request_id,
        payload_size,
        0
    };
    if (payload_size > SEAN_MAX_PACKET_SIZE ||
        sizeof(response) + payload_size > sizeof(packet))
        return;
    memcpy(packet, &response, sizeof(response));
    if (payload_size != 0 && payload != NULL)
        memcpy(packet + sizeof(response), payload, payload_size);
    (void)send(client, packet, sizeof(response) + payload_size, MSG_NOSIGNAL);
}

static void handle_client(int client, gid_t xum_group)
{
    struct ucred peer;
    socklen_t peer_size = sizeof(peer);
    if (getsockopt(client, SOL_SOCKET, SO_PEERCRED, &peer, &peer_size) < 0 ||
        peer_size != sizeof(peer) || peer.uid != 0 || peer.gid != xum_group) {
        syslog(LOG_WARNING, "denied non-XUM backend peer");
        reply(client, 0, SEAN_STATUS_DENIED, NULL, 0);
        return;
    }

    struct sean_request request = {0};
    ssize_t received = recv(client, &request, sizeof(request), MSG_TRUNC);
    if (received != (ssize_t)sizeof(request) ||
        request.size != sizeof(request) ||
        request.version != SEAN_PROTOCOL_VERSION ||
        request.request_id == 0 ||
        request.operation != SEAN_OP_READ_SYSTEM_INFO) {
        reply(client, request.request_id, SEAN_STATUS_INVALID, NULL, 0);
        return;
    }

    struct utsname system;
    char information[512];
    if (uname(&system) < 0) {
        reply(client, request.request_id, SEAN_STATUS_INTERNAL_ERROR, NULL, 0);
        return;
    }
    int length = snprintf(information, sizeof(information),
                          "Lenix; kernel=%s; release=%s; machine=%s",
                          system.sysname, system.release, system.machine);
    if (length < 0 || (size_t)length >= sizeof(information)) {
        reply(client, request.request_id, SEAN_STATUS_INTERNAL_ERROR, NULL, 0);
        return;
    }
    syslog(LOG_INFO, "served system-info request id=%llu",
           (unsigned long long)request.request_id);
    reply(client, request.request_id, SEAN_STATUS_OK,
          information, (uint32_t)length);
}

static int create_socket(void)
{
    const char *path = SEAN_SYSTEM_SOCKET;
    if (mkdir("/run/sean", 0700) < 0 && errno != EEXIST) return -1;
    struct stat directory;
    if (lstat("/run/sean", &directory) < 0 ||
        !S_ISDIR(directory.st_mode) || directory.st_uid != 0 ||
        (directory.st_mode & 0077) != 0) {
        errno = EPERM;
        return -1;
    }

    struct stat existing;
    if (lstat(path, &existing) == 0) {
        if (!S_ISSOCK(existing.st_mode) || existing.st_uid != 0) {
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
    memcpy(address.sun_path, path, strlen(path) + 1U);
    if (bind(server, (struct sockaddr *)&address, sizeof(address)) < 0 ||
        chmod(path, 0600) < 0 || listen(server, 16) < 0) {
        int saved = errno;
        close(server);
        unlink(path);
        errno = saved;
        return -1;
    }
    return server;
}

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        puts("Sean System Service 0.1.0");
        return 0;
    }
    if (argc != 1) {
        fputs("Usage: sean-system [--version]\n", stderr);
        return 64;
    }
    if (geteuid() != 0) {
        fputs("sean-system must run as root.\n", stderr);
        return 77;
    }

    struct group *group = getgrnam("mintosh-services");
    if (group == NULL) {
        fputs("sean-system: mintosh-services group is missing\n", stderr);
        return 1;
    }
    int server = create_socket();
    if (server < 0) {
        perror("sean-system: create socket");
        return 1;
    }

    openlog("sean-system", LOG_PID | LOG_NDELAY, LOG_AUTHPRIV);
    signal(SIGTERM, stop_server);
    signal(SIGINT, stop_server);
    signal(SIGPIPE, SIG_IGN);
    syslog(LOG_NOTICE, "Sean system service started");

    while (!stopping) {
        struct pollfd descriptor = { .fd = server, .events = POLLIN };
        int ready = poll(&descriptor, 1, 1000);
        if (ready < 0) {
            if (errno == EINTR) continue;
            syslog(LOG_ERR, "poll failed: %s", strerror(errno));
            break;
        }
        if (ready == 0) continue;
        int client = accept4(server, NULL, NULL, SOCK_CLOEXEC);
        if (client < 0) {
            if (errno == EINTR || errno == EAGAIN) continue;
            syslog(LOG_ERR, "accept failed: %s", strerror(errno));
            break;
        }
        struct timeval timeout = { .tv_sec = 2, .tv_usec = 0 };
        (void)setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        (void)setsockopt(client, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
        handle_client(client, group->gr_gid);
        close(client);
    }

    close(server);
    unlink(SEAN_SYSTEM_SOCKET);
    syslog(LOG_NOTICE, "Sean system service stopped");
    closelog();
    return 0;
}

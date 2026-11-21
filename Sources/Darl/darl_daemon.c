#define _GNU_SOURCE
#include "darl_protocol.h"
#include "libsys.h"
#include <errno.h>
#include <grp.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/un.h>
#include <unistd.h>

static volatile sig_atomic_t shutdown_requested;
static void request_shutdown(int signal_number) { (void)signal_number; shutdown_requested = 1; }

static int read_request(int descriptor, char *request, size_t capacity) {
    size_t used = 0;
    while (used + 1 < capacity) {
        char byte;
        ssize_t count = read(descriptor, &byte, 1);
        if (count < 0 && errno == EINTR) continue;
        if (count < 0) return -1;
        if (count == 0) break;
        if (byte == '\n' || byte == '\r') break;
        request[used++] = byte;
    }
    request[used] = '\0';
    if (used + 1 == capacity) { errno = EMSGSIZE; return -1; }
    return used ? 0 : -1;
}

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "--version") == 0) {
        puts("Darl 0.2.0");
        return 0;
    }
    if (getpid() == 1) {
        fputs("Darl is not the PID 1 init process.\n", stderr);
        return 2;
    }

    const char *runtime_directory = ms_runtime_directory();
    const char *socket_path = ms_darl_socket_path();
    if (!socket_path[0] || ms_mkdir_p(runtime_directory, 0755) < 0) {
        perror("darl: runtime directory");
        return 1;
    }
    int server = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (server < 0) { perror("darl: socket"); return 1; }
    struct sockaddr_un address = {.sun_family = AF_UNIX};
    if (strlen(socket_path) >= sizeof address.sun_path) {
        fputs("darl: socket path too long\n", stderr);
        close(server);
        return 1;
    }
    strcpy(address.sun_path, socket_path);
    (void)unlink(socket_path);
    if (bind(server, (struct sockaddr *)&address, sizeof address) < 0) {
        perror("darl: bind");
        close(server);
        return 1;
    }

    struct group *mintosh_group = getgrnam("mintosh");
    gid_t socket_group = mintosh_group ? mintosh_group->gr_gid : getegid();
    if (chown(socket_path, geteuid(), socket_group) < 0 ||
        chmod(socket_path, 0660) < 0 || listen(server, 16) < 0) {
        perror("darl: configure socket");
        close(server);
        unlink(socket_path);
        return 1;
    }

    signal(SIGTERM, request_shutdown);
    signal(SIGINT, request_shutdown);
    ms_log("darl", "info", "Darl started");
    while (!shutdown_requested) {
        fd_set readable;
        FD_ZERO(&readable);
        FD_SET(server, &readable);
        struct timeval timeout = {.tv_sec = 1};
        int ready = select(server + 1, &readable, NULL, NULL, &timeout);
        if (ready < 0) { if (errno == EINTR) continue; break; }
        if (ready == 0) continue;
        int client = accept4(server, NULL, NULL, SOCK_CLOEXEC);
        if (client < 0) { if (errno == EINTR) continue; break; }
        struct timeval client_timeout = {.tv_sec = 2, .tv_usec = 0};
        (void)setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, &client_timeout, sizeof client_timeout);
        char request[128] = {0}, response[256];
        if (read_request(client, request, sizeof request) == 0) {
            darl_format_response(request, response, sizeof response);
            (void)ms_write_all(client, response, strlen(response));
        } else {
            (void)ms_write_all(client, "ERROR invalid request\n", 22);
        }
        close(client);
    }
    close(server);
    unlink(socket_path);
    ms_log("darl", "info", "Darl stopped");
    return 0;
}

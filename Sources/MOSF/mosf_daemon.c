#define _GNU_SOURCE
#include "mosf_protocol.h"
#include "libsys.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

static volatile sig_atomic_t shutdown_requested;
static void request_shutdown(int signal_number) { (void)signal_number; shutdown_requested = 1; }

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "--version") == 0) { puts("MOSF 0.1.0"); return 0; }
    if (getpid() == 1) { fputs("MOSF is not the PID 1 init process.\n", stderr); return 2; }
    if (ms_mkdir_p(MINTOSH_RUNTIME_DIR, 0755) < 0) { perror("mosf: runtime directory"); return 1; }
    int server = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (server < 0) { perror("mosf: socket"); return 1; }
    struct sockaddr_un address = { .sun_family = AF_UNIX };
    strcpy(address.sun_path, MINTOSH_MOSF_SOCKET);
    (void)unlink(MINTOSH_MOSF_SOCKET);
    if (bind(server, (struct sockaddr *)&address, sizeof address) < 0 ||
        chmod(MINTOSH_MOSF_SOCKET, 0666) < 0 || listen(server, 16) < 0) {
        perror("mosf: listen"); close(server); unlink(MINTOSH_MOSF_SOCKET); return 1;
    }
    signal(SIGTERM, request_shutdown); signal(SIGINT, request_shutdown);
    ms_log("mosf", "info", "MOSF started");
    while (!shutdown_requested) {
        fd_set readable; FD_ZERO(&readable); FD_SET(server, &readable);
        struct timeval timeout = { .tv_sec = 1 };
        int ready = select(server + 1, &readable, NULL, NULL, &timeout);
        if (ready < 0) { if (errno == EINTR) continue; break; }
        if (ready == 0) continue;
        int client = accept4(server, NULL, NULL, SOCK_CLOEXEC);
        if (client < 0) { if (errno == EINTR) continue; break; }
        char request[128] = {0}, response[256];
        ssize_t received = read(client, request, sizeof request - 1);
        if (received > 0) {
            request[strcspn(request, "\r\n")] = '\0';
            mosf_format_response(request, response, sizeof response);
            (void)ms_write_all(client, response, strlen(response));
        }
        close(client);
    }
    close(server); unlink(MINTOSH_MOSF_SOCKET);
    ms_log("mosf", "info", "MOSF stopped");
    return 0;
}

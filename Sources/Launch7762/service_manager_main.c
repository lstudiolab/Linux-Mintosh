#define _GNU_SOURCE
#include "service_manager.h"
#include "service_socket.h"
#include "libsys.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

static volatile sig_atomic_t shutdown_requested;
static void request_shutdown(int signal_number) { (void)signal_number; shutdown_requested = 1; }

static void handle_client(MintoshServiceManager *manager, int descriptor) {
    char request[256] = {0};
    ssize_t length = read(descriptor, request, sizeof request - 1);
    if (length <= 0) return;
    request[strcspn(request, "\r\n")] = '\0';
    if (strcmp(request, "LIST") == 0) ms_services_list(manager, descriptor);
    else ms_services_control(manager, descriptor, request);
}

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "--version") == 0) { puts("launch7762 0.1.0"); return 0; }
    if (ms_mkdir_p(ms_runtime_directory(), 0755) < 0) { perror("launch7762: runtime directory"); return 1; }
    MintoshServiceManager manager = {0};
    if (ms_services_load(&manager, "/etc/mintosh/services") < 0) perror("launch7762: service directory");
    int server = ms_service_socket_open();
    if (server < 0) { perror("launch7762: control socket"); return 1; }
    signal(SIGTERM, request_shutdown);
    signal(SIGINT, request_shutdown);
    ms_log("launch7762", "info", "Mintosh service manager started");
    for (size_t index = 0; index < manager.count; ++index)
        if (manager.services[index].enabled) (void)ms_service_start(&manager.services[index]);
    while (!shutdown_requested) {
        fd_set readable;
        FD_ZERO(&readable); FD_SET(server, &readable);
        struct timeval timeout = { .tv_sec = 1 };
        int ready = select(server + 1, &readable, NULL, NULL, &timeout);
        if (ready > 0 && FD_ISSET(server, &readable)) {
            int client = accept4(server, NULL, NULL, SOCK_CLOEXEC);
            if (client >= 0) { handle_client(&manager, client); close(client); }
        } else if (ready < 0 && errno != EINTR) break;
        ms_services_reap(&manager, shutdown_requested);
    }
    ms_services_stop_all(&manager);
    for (int attempt = 0; attempt < 20; ++attempt) {
        ms_services_reap(&manager, 1);
        usleep(100000);
    }
    close(server);
    unlink(ms_launch_socket_path());
    ms_log("launch7762", "info", "Mintosh service manager stopped");
    return 0;
}

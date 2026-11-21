#define _GNU_SOURCE
#include "service_manager.h"
#include "service_socket.h"
#include "libsys.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

static volatile sig_atomic_t shutdown_requested;
static void request_shutdown(int signal_number) { (void)signal_number; shutdown_requested = 1; }

static void handle_client(MintoshServiceManager *manager, int descriptor) {
    struct ucred peer = {0};
    socklen_t peer_size = sizeof peer;
    int has_credentials = getsockopt(descriptor, SOL_SOCKET, SO_PEERCRED,
                                     &peer, &peer_size) == 0;
    char request[256];
    if (ms_read_line(descriptor, request, sizeof request, 2000) < 0) {
        (void)ms_write_all(descriptor, "ERROR invalid request\n",
                           sizeof("ERROR invalid request\n") - 1);
        return;
    }
    if (strcmp(request, "LIST") == 0) {
        ms_services_list(manager, descriptor);
        return;
    }
    int privileged = has_credentials &&
        (peer.uid == 0 || peer.uid == geteuid());
    ms_services_control(manager, descriptor, request, privileged);
}

static int running_services(const MintoshServiceManager *manager) {
    for (size_t index = 0; index < manager->count; ++index)
        if (manager->services[index].process_id > 0) return 1;
    return 0;
}

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "--version") == 0) { puts("launch7762 0.2.0"); return 0; }
    if (ms_mkdir_p(ms_runtime_directory(), 0755) < 0) { perror("launch7762: runtime directory"); return 1; }
    const char *service_directory = getenv("MINTOSH_SERVICE_DIR");
    if (!service_directory || !service_directory[0]) service_directory = "/etc/mintosh/services";
    MintoshServiceManager manager = {0};
    if (ms_services_load(&manager, service_directory) < 0) perror("launch7762: service directory");
    int server = ms_service_socket_open();
    if (server < 0) { perror("launch7762: control socket"); return 1; }
    signal(SIGTERM, request_shutdown);
    signal(SIGINT, request_shutdown);
    signal(SIGPIPE, SIG_IGN);
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
        ms_services_reap(&manager, 0);
    }
    ms_services_stop_all(&manager);
    for (int attempt = 0; attempt < 60 && running_services(&manager); ++attempt) {
        ms_services_reap(&manager, 1);
        usleep(100000);
    }
    if (running_services(&manager)) {
        for (size_t index = 0; index < manager.count; ++index) {
            MintoshService *service = &manager.services[index];
            if (service->process_id > 0) {
                (void)kill(-service->process_id, SIGKILL);
                (void)kill(service->process_id, SIGKILL);
            }
        }
        for (int attempt = 0; attempt < 10 && running_services(&manager); ++attempt) {
            ms_services_reap(&manager, 1);
            usleep(100000);
        }
    }
    close(server);
    unlink(ms_launch_socket_path());
    ms_log("launch7762", "info", "Mintosh service manager stopped");
    return 0;
}

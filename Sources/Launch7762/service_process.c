#define _GNU_SOURCE
#include "service_manager.h"
#include "libsys.h"
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

int ms_service_start(MintoshService *service) {
    if (!service || service->process_id > 0 || !service->command[0]) return 0;
    pid_t child = fork();
    if (child < 0) { ms_log("launch7762", "error", "unable to fork service process"); return -1; }
    if (child == 0) {
        if (setsid() < 0) _exit(126);
        execl("/usr/bin/launch511", "launch511", "--", "/bin/sh", "-c", service->command, (char *)NULL);
        _exit(127);
    }
    service->process_id = child;
    char message[160];
    snprintf(message, sizeof message, "started %s with pid %ld", service->name, (long)child);
    ms_log("launch7762", "info", message);
    return 0;
}

void ms_service_stop(MintoshService *service) {
    if (!service) return;
    service->enabled = 0;
    if (service->process_id > 0) {
        (void)kill(-service->process_id, SIGTERM);
        (void)kill(service->process_id, SIGTERM);
    }
}

void ms_services_reap(MintoshServiceManager *manager, int shutting_down) {
    int status;
    pid_t child;
    while ((child = waitpid(-1, &status, WNOHANG)) > 0) {
        for (size_t index = 0; index < manager->count; ++index) {
            MintoshService *service = &manager->services[index];
            if (service->process_id != child) continue;
            service->process_id = 0;
            char message[160];
            snprintf(message, sizeof message, "%s exited (wait status %d)", service->name, status);
            ms_log("launch7762", "warning", message);
            if (service->restart_on_exit && service->enabled && !shutting_down)
                (void)ms_service_start(service);
            break;
        }
    }
}

void ms_services_stop_all(MintoshServiceManager *manager) {
    for (size_t index = 0; index < manager->count; ++index) ms_service_stop(&manager->services[index]);
}

#define _GNU_SOURCE
#include "service_manager.h"
#include "libsys.h"
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define SERVICE_STOP_GRACE_SECONDS 3
#define SERVICE_RESTART_DELAY_SECONDS 1

static struct timespec monotonic_now(void) {
    struct timespec now = {0};
    (void)clock_gettime(CLOCK_MONOTONIC, &now);
    return now;
}

static struct timespec add_seconds(struct timespec value, time_t seconds) {
    value.tv_sec += seconds;
    return value;
}

static int time_reached(struct timespec now, struct timespec deadline) {
    return now.tv_sec > deadline.tv_sec ||
           (now.tv_sec == deadline.tv_sec && now.tv_nsec >= deadline.tv_nsec);
}

int ms_service_start(MintoshService *service) {
    if (!service || service->process_id > 0 || !service->command[0]) return 0;
    pid_t child = fork();
    if (child < 0) { ms_log("launch7762", "error", "unable to fork service process"); return -1; }
    if (child == 0) {
        if (setsid() < 0) _exit(126);
        execl("/usr/bin/launch511", "launch511", "--", "/bin/sh", "-c",
              service->command, (char *)NULL);
        _exit(127);
    }
    service->process_id = child;
    service->restart_pending = 0;
    service->kill_sent = 0;
    service->stop_deadline = (struct timespec){0};
    char message[160];
    snprintf(message, sizeof message, "started %s with pid %ld",
             service->name, (long)child);
    ms_log("launch7762", "info", message);
    return 0;
}

void ms_service_stop(MintoshService *service) {
    if (!service) return;
    service->enabled = 0;
    service->restart_pending = 0;
    if (service->process_id > 0) {
        (void)kill(-service->process_id, SIGTERM);
        (void)kill(service->process_id, SIGTERM);
        service->stop_deadline = add_seconds(monotonic_now(), SERVICE_STOP_GRACE_SECONDS);
        service->kill_sent = 0;
    }
}

static void schedule_restart(MintoshService *service, struct timespec now) {
    service->restart_pending = 1;
    service->restart_at = add_seconds(now, SERVICE_RESTART_DELAY_SECONDS);
}

void ms_services_reap(MintoshServiceManager *manager, int shutting_down) {
    struct timespec now = monotonic_now();
    int status;
    pid_t child;
    while ((child = waitpid(-1, &status, WNOHANG)) > 0) {
        for (size_t index = 0; index < manager->count; ++index) {
            MintoshService *service = &manager->services[index];
            if (service->process_id != child) continue;
            service->process_id = 0;
            service->stop_deadline = (struct timespec){0};
            service->kill_sent = 0;
            char message[160];
            snprintf(message, sizeof message, "%s exited (wait status %d)",
                     service->name, status);
            ms_log("launch7762", "warning", message);

            int failed = !WIFEXITED(status) || WEXITSTATUS(status) != 0;
            int should_restart = service->enabled && !shutting_down &&
                (service->restart_policy == MINTOSH_RESTART_ALWAYS ||
                 (service->restart_policy == MINTOSH_RESTART_ON_FAILURE && failed));
            if (should_restart) schedule_restart(service, now);
            break;
        }
    }

    for (size_t index = 0; index < manager->count; ++index) {
        MintoshService *service = &manager->services[index];
        if (service->process_id > 0 && service->stop_deadline.tv_sec != 0 &&
            !service->kill_sent && time_reached(now, service->stop_deadline)) {
            (void)kill(-service->process_id, SIGKILL);
            (void)kill(service->process_id, SIGKILL);
            service->kill_sent = 1;
            ms_log("launch7762", "warning", "service exceeded stop grace period; sent SIGKILL");
        }
        if (service->process_id == 0 && service->restart_pending &&
            service->enabled && !shutting_down && time_reached(now, service->restart_at)) {
            service->restart_pending = 0;
            (void)ms_service_start(service);
        }
    }
}

void ms_services_stop_all(MintoshServiceManager *manager) {
    for (size_t index = 0; index < manager->count; ++index)
        ms_service_stop(&manager->services[index]);
}

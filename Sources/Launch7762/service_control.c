#include "service_manager.h"
#include "libsys.h"
#include <stdio.h>
#include <string.h>

static void send_text(int descriptor, const char *text) {
    (void)ms_write_all(descriptor, text, strlen(text));
}

void ms_services_list(const MintoshServiceManager *manager, int descriptor) {
    char line[192];
    for (size_t index = 0; index < manager->count; ++index) {
        const MintoshService *service = &manager->services[index];
        const char *state = service->process_id > 0 ? "running" :
                            service->restart_pending ? "restarting" : "stopped";
        snprintf(line, sizeof line, "%s %s pid=%ld restarts=%u\n",
                 service->name, state, (long)service->process_id,
                 service->restart_count);
        send_text(descriptor, line);
    }
    send_text(descriptor, "END\n");
}

void ms_services_control(MintoshServiceManager *manager, int descriptor,
                         const char *request, int privileged) {
    char copy[256], *save = NULL;
    if (!request || strlen(request) >= sizeof copy) {
        send_text(descriptor, "ERROR invalid command\n");
        return;
    }
    strcpy(copy, request);
    char *action = strtok_r(copy, " \t", &save);
    char *name = strtok_r(NULL, " \t", &save);
    char *extra = strtok_r(NULL, " \t", &save);
    if (!action || !name || extra) {
        send_text(descriptor, "ERROR invalid command\n");
        return;
    }
    if (!privileged) {
        send_text(descriptor, "ERROR permission denied\n");
        return;
    }
    MintoshService *service = ms_services_find(manager, name);
    if (!service) { send_text(descriptor, "ERROR unknown service\n"); return; }
    if (strcmp(action, "START") == 0) {
        service->enabled = 1;
        send_text(descriptor, ms_service_start(service) == 0 ? "OK\n" : "ERROR start failed\n");
    } else if (strcmp(action, "STOP") == 0) {
        ms_service_stop(service);
        send_text(descriptor, "OK\n");
    } else if (strcmp(action, "RESTART") == 0) {
        service->enabled = 1;
        service->restart_count = 0;
        if (service->process_id > 0) {
            ms_service_stop(service);
            service->enabled = 1;
            service->restart_pending = 1;
            service->restart_at = (struct timespec){0};
            send_text(descriptor, "OK restart scheduled\n");
        } else {
            send_text(descriptor, ms_service_start(service) == 0 ? "OK\n" : "ERROR restart failed\n");
        }
    } else send_text(descriptor, "ERROR unsupported command\n");
}

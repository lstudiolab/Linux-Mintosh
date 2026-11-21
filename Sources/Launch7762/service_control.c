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
        snprintf(line, sizeof line, "%s %s pid=%ld\n", service->name,
                 service->process_id > 0 ? "running" : "stopped", (long)service->process_id);
        send_text(descriptor, line);
    }
    send_text(descriptor, "END\n");
}

void ms_services_control(MintoshServiceManager *manager, int descriptor, const char *request) {
    char action[24] = {0}, name[MINTOSH_SERVICE_NAME_MAX] = {0};
    if (sscanf(request, "%23s %63s", action, name) != 2) {
        send_text(descriptor, "ERROR invalid command\n"); return;
    }
    MintoshService *service = ms_services_find(manager, name);
    if (!service) { send_text(descriptor, "ERROR unknown service\n"); return; }
    if (strcmp(action, "START") == 0) {
        service->enabled = 1;
        send_text(descriptor, ms_service_start(service) == 0 ? "OK\n" : "ERROR start failed\n");
    } else if (strcmp(action, "STOP") == 0) {
        ms_service_stop(service); send_text(descriptor, "OK\n");
    } else if (strcmp(action, "RESTART") == 0) {
        service->enabled = 1;
        if (service->process_id > 0) {
            /* The reaper observes the exit and applies the configured restart policy. */
            ms_service_stop(service);
            service->enabled = 1;
            send_text(descriptor, "OK restart scheduled by service policy\n");
        } else {
            send_text(descriptor, ms_service_start(service) == 0 ? "OK\n" : "ERROR restart failed\n");
        }
    } else send_text(descriptor, "ERROR unsupported command\n");
}

#include "service_manager.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int ms_service_start(MintoshService *service) { (void)service; return 0; }
void ms_service_stop(MintoshService *service) {
    service->enabled = 0;
    service->restart_pending = 0;
}

MintoshService *ms_services_find(MintoshServiceManager *manager, const char *name) {
    if (!manager || !name) return NULL;
    for (size_t index = 0; index < manager->count; ++index)
        if (strcmp(manager->services[index].name, name) == 0) return &manager->services[index];
    return NULL;
}

static int expect_response(int descriptor, const char *expected) {
    char response[128] = {0};
    ssize_t count = read(descriptor, response, sizeof response - 1);
    return count > 0 && strcmp(response, expected) == 0;
}

int main(void) {
    MintoshServiceManager manager = {0};
    manager.count = 1;
    strcpy(manager.services[0].name, "protected");
    strcpy(manager.services[0].command, "/bin/true");
    manager.services[0].enabled = 1;
    int channel[2];
    if (pipe(channel) < 0) return 1;

    ms_services_control(&manager, channel[1], "STOP protected", 0);
    close(channel[1]);
    int denied = expect_response(channel[0], "ERROR permission denied\n");
    close(channel[0]);
    if (!denied || !manager.services[0].enabled) {
        fputs("service-control: unauthorized STOP was not rejected\n", stderr);
        return 1;
    }

    if (pipe(channel) < 0) return 1;
    ms_services_control(&manager, channel[1], "STOP protected", 1);
    close(channel[1]);
    int accepted = expect_response(channel[0], "OK\n");
    close(channel[0]);
    if (!accepted || manager.services[0].enabled) {
        fputs("service-control: authorized STOP was not applied\n", stderr);
        return 1;
    }
    puts("Service control authorization tests passed.");
    return 0;
}

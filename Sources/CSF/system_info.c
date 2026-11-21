#include "csf.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

int csf_system_ping(void) {
    char response[128] = {0};
    if (csf_request("PING", response, sizeof response) < 0) return -1;
    if (strncmp(response, "OK Darl ", 8) != 0) { errno = EPROTO; return -1; }
    return 0;
}

int csf_system_version(char *response, size_t capacity) {
    if (!response || capacity == 0) { errno = EINVAL; return -1; }
    if (csf_request("VERSION", response, capacity) < 0) return -1;
    if (strncmp(response, "Mintosh ", 8) != 0) { errno = EPROTO; return -1; }
    return 0;
}

int csf_system_status(CSFSystemStatus *status) {
    if (!status) { errno = EINVAL; return -1; }
    char response[128] = {0};
    if (csf_request("STATUS", response, sizeof response) < 0) return -1;
    if (strncmp(response, "Darl=running", 12) != 0) {
        errno = EPROTO;
        return -1;
    }
    char version[128] = {0};
    if (csf_system_version(version, sizeof version) < 0) return -1;
    char mintosh_version[CSF_VERSION_MAX], darl_version[CSF_VERSION_MAX];
    if (sscanf(version, "Mintosh %31[^;]; Darl %31s",
               mintosh_version, darl_version) != 2) {
        errno = EPROTO;
        return -1;
    }
    *status = (CSFSystemStatus){.darl_running = 1};
    snprintf(status->mintosh_version, sizeof status->mintosh_version,
             "%s", mintosh_version);
    snprintf(status->darl_version, sizeof status->darl_version,
             "%s", darl_version);
    char services[4096] = {0};
    status->launch_manager_running = csf_service_list(services, sizeof services) == 0;
    return 0;
}

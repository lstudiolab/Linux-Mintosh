#include "csf.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

int csf_system_ping(void) {
    char response[128] = {0};
    if (csf_request("PING", response, sizeof response) < 0) return -1;
    return strncmp(response, "OK Darl ", 8) == 0 ? 0 : (errno = EPROTO, -1);
}

int csf_system_version(char *response, size_t capacity) {
    if (!response || capacity == 0) { errno = EINVAL; return -1; }
    if (csf_request("VERSION", response, capacity) < 0) return -1;
    return strncmp(response, "Mintosh ", 8) == 0 ? 0 : (errno = EPROTO, -1);
}

int csf_system_status(CSFSystemStatus *status) {
    if (!status) { errno = EINVAL; return -1; }
    char response[128] = {0};
    if (csf_request("STATUS", response, sizeof response) < 0) return -1;
    if (strncmp(response, "Darl=running", 12) != 0) {
        errno = EPROTO;
        return -1;
    }
    *status = (CSFSystemStatus){
        .darl_running = 1
    };
    snprintf(status->mintosh_version, sizeof status->mintosh_version, "%s", "0.1.0");
    snprintf(status->darl_version, sizeof status->darl_version, "%s", "0.2.0");
    return 0;
}

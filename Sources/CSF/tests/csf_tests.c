#include "csf.h"
#include "libsys.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

static char last_request[128];

const char *ms_launch_socket_path(void) { return "/run/mintosh/launch.sock"; }
int ms_send_command(const char *path, const char *command,
                    char *response, size_t capacity) {
    assert(strcmp(path, "/run/mintosh/launch.sock") == 0);
    snprintf(last_request, sizeof last_request, "%s", command);
    snprintf(response, capacity, "END\n");
    return 0;
}
int csf_request(const char *request, char *response, size_t capacity) {
    if (strcmp(request, "PING") == 0) snprintf(response, capacity, "OK Darl 0.2.0\n");
    else if (strcmp(request, "VERSION") == 0)
        snprintf(response, capacity, "Mintosh 0.1.0; Darl 0.2.0\n");
    else if (strcmp(request, "STATUS") == 0) snprintf(response, capacity, "Darl=running\n");
    else { errno = EPROTO; return -1; }
    return 0;
}

int main(void) {
    assert(csf_service_name_is_valid("darl"));
    assert(csf_service_name_is_valid("user-session_2"));
    assert(!csf_service_name_is_valid(""));
    assert(!csf_service_name_is_valid("../darl"));
    assert(!csf_service_name_is_valid("darl service"));
    assert(!csf_service_name_is_valid("Darl"));
    assert(!csf_service_name_is_valid("darl;id"));
    assert(!csf_service_name_is_valid("a-name-that-is-longer-than-sixty-three-characters-and-invalid"));

    assert(csf_service_control("start", "darl") == 0);
    assert(strcmp(last_request, "START darl") == 0);
    assert(csf_service_control("stop", "darl") == 0);
    assert(strcmp(last_request, "STOP darl") == 0);
    assert(csf_service_control("restart", "darl") == 0);
    assert(strcmp(last_request, "RESTART darl") == 0);
    assert(csf_service_control("start", "../darl") < 0);
    assert(csf_service_control("launch", "darl") < 0);

    assert(csf_system_ping() == 0);
    char version[128];
    assert(csf_system_version(version, sizeof version) == 0);
    assert(strcmp(version, "Mintosh 0.1.0; Darl 0.2.0\n") == 0);
    CSFSystemStatus status;
    assert(csf_system_status(&status) == 0);
    assert(status.darl_running && status.launch_manager_running);
    assert(strcmp(status.mintosh_version, "0.1.0") == 0);
    assert(strcmp(status.darl_version, "0.2.0") == 0);
    return 0;
}

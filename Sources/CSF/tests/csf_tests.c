#include "csf.h"
#include "libsys.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static char last_request[128];

const char *ms_launch_socket_path(void) { return "/run/mintosh/launch.sock"; }
int ms_send_command(const char *path, const char *command,
                    char *response, size_t capacity) {
    assert(strcmp(path, "/run/mintosh/launch.sock") == 0);
    snprintf(last_request, sizeof last_request, "%s", command);
    snprintf(response, capacity, "OK\n");
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
    return 0;
}

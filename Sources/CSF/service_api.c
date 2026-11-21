#include "csf.h"
#include "libsys.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

int csf_service_name_is_valid(const char *name) {
    if (!name || !name[0] || strlen(name) >= 64) return 0;
    for (const unsigned char *cursor = (const unsigned char *)name; *cursor; ++cursor)
        if (!((*cursor >= 'a' && *cursor <= 'z') ||
              (*cursor >= '0' && *cursor <= '9') ||
              *cursor == '-' || *cursor == '_')) return 0;
    return 1;
}

int csf_service_list(char *response, size_t capacity) {
    if (!response || capacity == 0) { errno = EINVAL; return -1; }
    return ms_send_command(ms_launch_socket_path(), "LIST", response, capacity);
}

int csf_service_control(const char *action, const char *name) {
    if (!action || !name || !csf_service_name_is_valid(name)) {
        errno = EINVAL;
        return -1;
    }
    if (strcmp(action, "start") != 0 &&
        strcmp(action, "stop") != 0 &&
        strcmp(action, "restart") != 0) {
        errno = EINVAL;
        return -1;
    }
    char request[128], response[256] = {0};
    int length = snprintf(request, sizeof request, "%s %s", action, name);
    if (length < 0 || (size_t)length >= sizeof request) {
        errno = EOVERFLOW;
        return -1;
    }
    if (ms_send_command(ms_launch_socket_path(), request,
                        response, sizeof response) < 0) return -1;
    if (strncmp(response, "OK ", 3) != 0) {
        errno = EPERM;
        return -1;
    }
    return 0;
}

#include <stdio.h>
#include <string.h>

void darl_format_response(const char *request, char *response, size_t capacity) {
    if (!response || capacity == 0) return;

    if (!request) {
        (void)snprintf(response, capacity, "ERROR invalid request\n");
    } else if (strcmp(request, "PING") == 0) {
        (void)snprintf(response, capacity, "OK Darl 0.2.0\n");
    } else if (strcmp(request, "VERSION") == 0) {
        (void)snprintf(response, capacity, "Mintosh 0.1.0; Darl 0.2.0\n");
    } else if (strcmp(request, "STATUS") == 0) {
        (void)snprintf(response, capacity, "Darl=running\n");
    } else {
        (void)snprintf(response, capacity, "ERROR unsupported command\n");
    }
}

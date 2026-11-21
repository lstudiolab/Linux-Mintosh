#include <stdio.h>
#include <string.h>

void mosf_format_response(const char *request, char *response, size_t capacity) {
    if (strcmp(request, "PING") == 0) snprintf(response, capacity, "OK MOSF 0.1.0\n");
    else if (strcmp(request, "VERSION") == 0) snprintf(response, capacity, "Mintosh 0.1.0; MOSF 0.1.0\n");
    else if (strcmp(request, "STATUS") == 0) snprintf(response, capacity, "MOSF=running\n");
    else snprintf(response, capacity, "ERROR unsupported command\n");
}

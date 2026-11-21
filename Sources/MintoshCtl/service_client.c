#include "libsys.h"
int mintosh_service_request(const char *command, char *response, size_t capacity) {
    return ms_send_command(MINTOSH_LAUNCH_SOCKET, command, response, capacity);
}

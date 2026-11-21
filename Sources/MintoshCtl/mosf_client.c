#include "libsys.h"
int mintosh_mosf_request(const char *command, char *response, size_t capacity) {
    return ms_send_command(MINTOSH_MOSF_SOCKET, command, response, capacity);
}

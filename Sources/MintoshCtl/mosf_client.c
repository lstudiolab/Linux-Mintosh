#include "libsys.h"
int mintosh_mosf_request(const char *command, char *response, size_t capacity) {
    return ms_send_command(ms_mosf_socket_path(), command, response, capacity);
}

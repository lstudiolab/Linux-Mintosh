#include "libsys.h"
int mintosh_darl_request(const char *command, char *response, size_t capacity) {
    return ms_send_command(ms_darl_socket_path(), command, response, capacity);
}

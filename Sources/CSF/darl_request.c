#include "csf.h"
#include "libsys.h"
int csf_request(const char *request, char *response, size_t capacity) {
    return ms_send_command(ms_darl_socket_path(), request, response, capacity);
}

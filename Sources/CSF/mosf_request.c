#include "csf.h"
#include "libsys.h"
int csf_request(const char *request, char *response, size_t capacity) {
    return ms_send_command(MINTOSH_MOSF_SOCKET, request, response, capacity);
}

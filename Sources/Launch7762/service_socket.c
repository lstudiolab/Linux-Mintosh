#define _GNU_SOURCE
#include "service_manager.h"
#include "libsys.h"
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

int ms_service_socket_open(void) {
    int descriptor = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (descriptor < 0) return -1;
    struct sockaddr_un address = { .sun_family = AF_UNIX };
    strcpy(address.sun_path, MINTOSH_LAUNCH_SOCKET);
    (void)unlink(MINTOSH_LAUNCH_SOCKET);
    if (bind(descriptor, (struct sockaddr *)&address, sizeof address) < 0 ||
        chmod(MINTOSH_LAUNCH_SOCKET, 0660) < 0 || listen(descriptor, 16) < 0) {
        int saved_errno = errno; close(descriptor); unlink(MINTOSH_LAUNCH_SOCKET); errno = saved_errno; return -1;
    }
    return descriptor;
}

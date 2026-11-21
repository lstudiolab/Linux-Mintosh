#define _GNU_SOURCE
#include "libsys.h"
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int ms_send_command(const char *socket_path, const char *command, char *reply, size_t capacity) {
    if (!socket_path || !command || !reply || capacity < 2) { errno = EINVAL; return -1; }
    struct sockaddr_un address = { .sun_family = AF_UNIX };
    size_t path_length = strlen(socket_path);
    if (path_length >= sizeof address.sun_path) { errno = ENAMETOOLONG; return -1; }
    memcpy(address.sun_path, socket_path, path_length + 1);
    int descriptor = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (descriptor < 0) return -1;
    if (connect(descriptor, (struct sockaddr *)&address, sizeof address) < 0 ||
        ms_write_all(descriptor, command, strlen(command)) < 0 ||
        ms_write_all(descriptor, "\n", 1) < 0) {
        int saved_errno = errno; close(descriptor); errno = saved_errno; return -1;
    }
    ssize_t received;
    do { received = read(descriptor, reply, capacity - 1); } while (received < 0 && errno == EINTR);
    int saved_errno = errno;
    close(descriptor);
    errno = saved_errno;
    if (received < 0) return -1;
    reply[received] = '\0';
    return (int)received;
}

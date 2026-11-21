#define _GNU_SOURCE
#include "libsys.h"
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <unistd.h>

int ms_send_command(const char *socket_path, const char *command, char *reply, size_t capacity) {
    if (!socket_path || !command || !reply || capacity < 2) {
        errno = EINVAL;
        return -1;
    }
    struct sockaddr_un address = {.sun_family = AF_UNIX};
    size_t path_length = strlen(socket_path);
    if (path_length >= sizeof address.sun_path) {
        errno = ENAMETOOLONG;
        return -1;
    }
    memcpy(address.sun_path, socket_path, path_length + 1);

    int descriptor = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (descriptor < 0) return -1;
    struct timeval timeout = {.tv_sec = 3, .tv_usec = 0};
    if (setsockopt(descriptor, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0 ||
        setsockopt(descriptor, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof timeout) < 0 ||
        connect(descriptor, (struct sockaddr *)&address, sizeof address) < 0 ||
        ms_write_all(descriptor, command, strlen(command)) < 0 ||
        ms_write_all(descriptor, "\n", 1) < 0) {
        int saved_errno = errno;
        close(descriptor);
        errno = saved_errno;
        return -1;
    }

    size_t total = 0;
    while (total < capacity - 1) {
        ssize_t received = read(descriptor, reply + total, capacity - 1 - total);
        if (received < 0 && errno == EINTR) continue;
        if (received < 0) {
            int saved_errno = errno;
            close(descriptor);
            errno = saved_errno;
            return -1;
        }
        if (received == 0) break;
        total += (size_t)received;
    }
    close(descriptor);
    reply[total] = '\0';
    if (total == capacity - 1) {
        errno = EMSGSIZE;
        return -1;
    }
    return (int)total;
}

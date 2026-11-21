#define _POSIX_C_SOURCE 200809L
#include "libsys.h"
#include <errno.h>
#include <poll.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

int ms_read_line(int descriptor, char *buffer, size_t capacity, int timeout_ms) {
    if (descriptor < 0 || !buffer || capacity < 2 || timeout_ms < 0) {
        errno = EINVAL;
        return -1;
    }
    struct timespec deadline;
    if (clock_gettime(CLOCK_MONOTONIC, &deadline) < 0) return -1;
    deadline.tv_sec += timeout_ms / 1000;
    deadline.tv_nsec += (long)(timeout_ms % 1000) * 1000000L;
    if (deadline.tv_nsec >= 1000000000L) {
        ++deadline.tv_sec;
        deadline.tv_nsec -= 1000000000L;
    }

    size_t used = 0;
    while (used + 1 < capacity) {
        struct timespec now;
        if (clock_gettime(CLOCK_MONOTONIC, &now) < 0) return -1;
        int64_t remaining = (int64_t)(deadline.tv_sec - now.tv_sec) * 1000 +
                            (deadline.tv_nsec - now.tv_nsec) / 1000000;
        if (remaining <= 0) { errno = ETIMEDOUT; return -1; }

        struct pollfd ready = {.fd = descriptor, .events = POLLIN};
        int result = poll(&ready, 1, (int)remaining);
        if (result < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (result == 0) { errno = ETIMEDOUT; return -1; }
        if (ready.revents & POLLNVAL) { errno = EBADF; return -1; }

        char byte;
        ssize_t count = read(descriptor, &byte, 1);
        if (count < 0) {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) continue;
            return -1;
        }
        if (count == 0) { errno = used ? EPROTO : ECONNRESET; return -1; }
        if (byte == '\0') { errno = EINVAL; return -1; }
        if (byte == '\n' || byte == '\r') {
            buffer[used] = '\0';
            if (!used) { errno = EINVAL; return -1; }
            return (int)used;
        }
        buffer[used++] = byte;
    }
    buffer[used] = '\0';
    errno = EMSGSIZE;
    return -1;
}

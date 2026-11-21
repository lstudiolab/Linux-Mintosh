#include "libsys.h"
#include <errno.h>
#include <unistd.h>

int ms_write_all(int descriptor, const void *data, size_t length) {
    if (descriptor < 0 || (length && !data)) { errno = EINVAL; return -1; }
    const char *cursor = data;
    while (length) {
        ssize_t written = write(descriptor, cursor, length);
        if (written < 0) { if (errno == EINTR) continue; return -1; }
        if (written == 0) { errno = EIO; return -1; }
        cursor += (size_t)written;
        length -= (size_t)written;
    }
    return 0;
}

#include "libsys.h"
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

int ms_mkdir_p(const char *path, mode_t mode) {
    char buffer[4096];
    size_t length = strlen(path);
    if (length == 0 || length >= sizeof buffer) { errno = ENAMETOOLONG; return -1; }
    memcpy(buffer, path, length + 1);
    for (char *cursor = buffer + 1; *cursor; ++cursor) {
        if (*cursor != '/') continue;
        *cursor = '\0';
        if (mkdir(buffer, mode) < 0 && errno != EEXIST) return -1;
        *cursor = '/';
    }
    return mkdir(buffer, mode) < 0 && errno != EEXIST ? -1 : 0;
}

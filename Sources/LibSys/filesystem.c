#define _GNU_SOURCE
#include "libsys.h"
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int ms_mkdir_p(const char *path, mode_t mode) {
    if (!path || !path[0] || (mode & ~0777) != 0) {
        errno = EINVAL;
        return -1;
    }
    size_t length = strlen(path);
    if (length >= 4096) { errno = ENAMETOOLONG; return -1; }
    char *copy = strdup(path);
    if (!copy) return -1;

    int current = open(path[0] == '/' ? "/" : ".",
                       O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (current < 0) { free(copy); return -1; }

    char *save = NULL;
    for (char *part = strtok_r(copy, "/", &save);
         part; part = strtok_r(NULL, "/", &save)) {
        if (strcmp(part, ".") == 0) continue;
        if (strcmp(part, "..") == 0) {
            errno = EINVAL;
            close(current);
            free(copy);
            return -1;
        }
        if (mkdirat(current, part, mode) < 0 && errno != EEXIST) {
            int saved = errno; close(current); free(copy); errno = saved; return -1;
        }
        int next = openat(current, part,
                          O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
        if (next < 0) {
            int saved = errno; close(current); free(copy); errno = saved; return -1;
        }
        struct stat metadata;
        if (fstat(next, &metadata) < 0 || !S_ISDIR(metadata.st_mode)) {
            int saved = errno ? errno : ENOTDIR;
            close(next); close(current); free(copy); errno = saved; return -1;
        }
        close(current);
        current = next;
    }
    close(current);
    free(copy);
    return 0;
}

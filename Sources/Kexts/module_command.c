#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int kexts_execute(char *const arguments[]) {
    const char *path = NULL;
    if (strcmp(arguments[0], "lsmod") == 0) path = "/usr/sbin/lsmod";
    else if (strcmp(arguments[0], "modprobe") == 0) path = "/usr/sbin/modprobe";
    if (!path) {
        fputs("kextctl: refusing unknown module utility\n", stderr);
        return 126;
    }
    execv(path, arguments);
    int error_number = errno;
    fprintf(stderr, "kextctl: %s: %s\n", path, strerror(error_number));
    return error_number == ENOENT ? 127 : 1;
}

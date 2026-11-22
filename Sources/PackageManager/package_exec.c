#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int mstore_execute(char *const arguments[]) {
    const char *program = arguments[0];
    const char *path = NULL;
    if (strcmp(program, "apt-get") == 0) path = "/usr/bin/apt-get";
    else if (strcmp(program, "apt-cache") == 0) path = "/usr/bin/apt-cache";
    else if (strcmp(program, "dpkg-query") == 0) path = "/usr/bin/dpkg-query";
    else if (strcmp(program, "flatpak") == 0) path = "/usr/bin/flatpak";
    if (!path) {
        fprintf(stderr, "mstore: refusing unknown system utility: %s\n", program);
        return 126;
    }
    execv(path, arguments);
    int error_number = errno;
    fprintf(stderr, "mstore: cannot run %s: %s\n", path, strerror(error_number));
    return error_number == ENOENT ? 127 : 126;
}

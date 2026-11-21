#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int mstore_execute(char *const arguments[]) {
    execvp(arguments[0], arguments);
    int error_number = errno;
    fprintf(stderr, "mstore: cannot run %s: %s\n",
            arguments[0], strerror(error_number));
    return error_number == ENOENT ? 127 : 126;
}

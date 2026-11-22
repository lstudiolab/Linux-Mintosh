#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static int core_is_ready(void) {
    pid_t child = fork();
    if (child < 0) return 0;
    if (child == 0) {
        char *const arguments[] = { "mintoshctl", "status", NULL };
        execv("/usr/bin/mintoshctl", arguments);
        _exit(127);
    }

    int status = 0;
    while (waitpid(child, &status, 0) < 0) {
        if (errno == EINTR) continue;
        return 0;
    }
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

int main(void) {
    const struct timespec retry_delay = { .tv_sec = 1, .tv_nsec = 0 };
    for (unsigned int attempt = 0; attempt < 10; ++attempt) {
        if (core_is_ready()) {
            puts("Mintosh Base is ready");
            return 0;
        }
        if (attempt + 1 < 10) {
            struct timespec remaining = retry_delay;
            while (nanosleep(&remaining, &remaining) < 0 && errno == EINTR) {}
        }
    }
    fputs("mintosh-ready: native core did not become ready within 10 seconds\n", stderr);
    return 1;
}

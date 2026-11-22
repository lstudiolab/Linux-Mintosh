#define _GNU_SOURCE
#include "service_manager.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
    MintoshService service = {0};
    strcpy(service.name, "trust-boundary-test");
    strcpy(service.command, "/tmp/mintosh-untrusted-executable");

    if (ms_service_start(&service) == 0 || errno != EPERM ||
        service.process_id != 0) {
        fputs("launch7762: accepted an executable from a writable path\n", stderr);
        return 1;
    }

    strcpy(service.command, "/usr/bin/true");
    if (ms_service_start(&service) < 0 || service.process_id <= 0) {
        fputs("launch7762: rejected a trusted system executable\n", stderr);
        return 1;
    }

    int status = 0;
    pid_t result;
    do {
        result = waitpid(service.process_id, &status, 0);
    } while (result < 0 && errno == EINTR);
    if (result != service.process_id || !WIFEXITED(status) ||
        WEXITSTATUS(status) != 0) {
        fputs("launch7762: trusted executable did not complete successfully\n", stderr);
        return 1;
    }

    puts("Service executable trust-boundary tests passed.");
    return 0;
}

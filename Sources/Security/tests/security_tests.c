#define _GNU_SOURCE
#include "permission_check.h"
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main(void) {
    char directory[] = "/tmp/mintosh-security-XXXXXX";
    assert(mkdtemp(directory));
    assert(chmod(directory, 0755) == 0);
    char good[256], bad[256], link[256];
    snprintf(good, sizeof good, "%s/good.service", directory);
    snprintf(bad, sizeof bad, "%s/bad.service", directory);
    snprintf(link, sizeof link, "%s/link.service", directory);

    int descriptor = open(good, O_WRONLY | O_CREAT | O_EXCL, 0644);
    assert(descriptor >= 0);
    assert(write(descriptor, "ExecStart=/bin/true\n", 20) == 20);
    close(descriptor);
    assert(chmod(good, 0644) == 0);
    assert(security_check_directory_permissions(directory, getuid()) == 0);
    assert(security_check_service_directory(directory, getuid()) == 0);

    descriptor = open(bad, O_WRONLY | O_CREAT | O_EXCL, 0666);
    assert(descriptor >= 0);
    close(descriptor);
    assert(chmod(bad, 0666) == 0);
    assert(symlink(good, link) == 0);
    assert(security_check_service_directory(directory, getuid()) == 2);

    unlink(link);
    unlink(bad);
    unlink(good);
    rmdir(directory);
    return 0;
}

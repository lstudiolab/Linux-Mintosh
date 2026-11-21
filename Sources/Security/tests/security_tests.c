#define _GNU_SOURCE
#include "permission_check.h"
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int main(void) {
    char directory[] = "/tmp/mintosh-security-XXXXXX";
    assert(mkdtemp(directory));
    assert(chmod(directory, 0755) == 0);
    char good[256], bad[256], link[256], grouped[256], socket_path[256];
    snprintf(good, sizeof good, "%s/good.service", directory);
    snprintf(bad, sizeof bad, "%s/bad.service", directory);
    snprintf(link, sizeof link, "%s/link.service", directory);
    snprintf(grouped, sizeof grouped, "%s/grouped.db", directory);
    snprintf(socket_path, sizeof socket_path, "%s/test.sock", directory);

    int descriptor = open(good, O_WRONLY | O_CREAT | O_EXCL, 0644);
    assert(descriptor >= 0);
    assert(write(descriptor, "ExecStart=/bin/true\n", 20) == 20);
    close(descriptor);
    assert(chmod(good, 0644) == 0);
    descriptor = open(grouped, O_WRONLY | O_CREAT | O_EXCL, 0600);
    assert(descriptor >= 0);
    close(descriptor);
    assert(chmod(grouped, 0600) == 0);
    assert(security_check_file_group_permissions(grouped, 0600, getuid(), getgid()) == 0);

    int socket_descriptor = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    assert(socket_descriptor >= 0);
    struct sockaddr_un address = {.sun_family = AF_UNIX};
    snprintf(address.sun_path, sizeof address.sun_path, "%s", socket_path);
    assert(bind(socket_descriptor, (struct sockaddr *)&address, sizeof address) == 0);
    assert(chmod(socket_path, 0600) == 0);
    assert(security_check_socket_permissions(socket_path, getuid(), getgid(), 0600) == 0);
    close(socket_descriptor);

    assert(security_check_directory_permissions(directory, getuid()) == 0);
    assert(security_check_service_directory(directory, getuid()) == 0);

    descriptor = open(bad, O_WRONLY | O_CREAT | O_EXCL, 0666);
    assert(descriptor >= 0);
    close(descriptor);
    assert(chmod(bad, 0666) == 0);
    assert(symlink(good, link) == 0);
    assert(security_check_service_directory(directory, getuid()) == 2);

    unlink(link);
    unlink(socket_path);
    unlink(grouped);
    unlink(bad);
    unlink(good);
    rmdir(directory);
    return 0;
}

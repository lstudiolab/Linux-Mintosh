#define _GNU_SOURCE
#include "service_manager.h"

#include <assert.h>
#include <fcntl.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static void write_service(const char *directory, const char *name,
                          const char *contents)
{
    char path[512];
    int length = snprintf(path, sizeof(path), "%s/%s.service", directory, name);
    assert(length > 0 && (size_t)length < sizeof(path));
    int descriptor = open(path, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0644);
    assert(descriptor >= 0);
    size_t size = strlen(contents);
    assert(write(descriptor, contents, size) == (ssize_t)size);
    assert(fchmod(descriptor, 0644) == 0);
    close(descriptor);
}

int main(void)
{
    char directory[] = "/tmp/mintosh-service-config-XXXXXX";
    assert(mkdtemp(directory) != NULL);
    assert(chmod(directory, 0755) == 0);

    write_service(directory, "xpu",
        "ExecStart=/usr/bin/xpu\nEnabled=yes\nRestart=always\n"
        "Group=root\nOrder=20\n");
    write_service(directory, "tarts",
        "ExecStart=/usr/bin/tarts-system\nEnabled=yes\nRestart=always\n"
        "Order=10\n");
    write_service(directory, "darl",
        "ExecStart=/usr/bin/darl\nEnabled=no\nRestart=on-failure\n"
        "Order=30\n");
    write_service(directory, "bad-order",
        "ExecStart=/usr/bin/false\nEnabled=yes\nOrder=-1\n");

    MintoshServiceManager manager = {0};
    assert(ms_services_load(&manager, directory) == 0);
    assert(manager.count == 3);
    assert(strcmp(manager.services[0].name, "tarts") == 0);
    assert(strcmp(manager.services[1].name, "xpu") == 0);
    assert(strcmp(manager.services[2].name, "darl") == 0);
    assert(manager.services[0].start_order == 10);
    assert(manager.services[1].start_order == 20);
    assert(manager.services[2].start_order == 30);

    struct group *services = getgrnam("root");
    assert(services != NULL);
    assert(manager.services[1].has_group == 1);
    assert(manager.services[1].group_id == services->gr_gid);
    assert(manager.services[0].has_group == 0);

    const char *names[] = {"xpu", "tarts", "darl", "bad-order"};
    for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); ++i) {
        char path[512];
        int length = snprintf(path, sizeof(path), "%s/%s.service", directory, names[i]);
        assert(length > 0 && (size_t)length < sizeof(path));
        assert(unlink(path) == 0);
    }
    assert(rmdir(directory) == 0);
    puts("Tarts service configuration tests passed.");
    return 0;
}

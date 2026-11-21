#define _GNU_SOURCE
#include "service_manager.h"
#include "libsys.h"
#include <errno.h>
#include <grp.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

int ms_service_socket_open(void) {
    int descriptor = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (descriptor < 0) return -1;
    struct sockaddr_un address = {.sun_family = AF_UNIX};
    const char *socket_path = ms_launch_socket_path();
    if (!socket_path[0] || strlen(socket_path) >= sizeof address.sun_path) {
        close(descriptor);
        errno = ENAMETOOLONG;
        return -1;
    }
    strcpy(address.sun_path, socket_path);
    (void)unlink(socket_path);
    if (bind(descriptor, (struct sockaddr *)&address, sizeof address) < 0) {
        int saved_errno = errno;
        close(descriptor);
        errno = saved_errno;
        return -1;
    }

    struct group *mintosh_group = getgrnam("mintosh");
    gid_t socket_group = mintosh_group ? mintosh_group->gr_gid : getegid();
    if (chown(socket_path, geteuid(), socket_group) < 0 ||
        chmod(socket_path, 0660) < 0 || listen(descriptor, 16) < 0) {
        int saved_errno = errno;
        close(descriptor);
        unlink(socket_path);
        errno = saved_errno;
        return -1;
    }
    return descriptor;
}

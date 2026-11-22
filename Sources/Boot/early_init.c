#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int ensure_directory(const char *path)
{
    if (mkdir(path, 0755) == 0 || errno == EEXIST)
        return 0;

    fprintf(stderr, "Mintosh init: cannot create %s: %s\n", path, strerror(errno));
    return -1;
}

static int mount_filesystem(const char *source, const char *target,
                            const char *type, unsigned long flags,
                            const char *options)
{
    if (mount(source, target, type, flags, options) == 0 || errno == EBUSY)
        return 0;

    fprintf(stderr, "Mintosh init: cannot mount %s on %s: %s\n",
            type, target, strerror(errno));
    return -1;
}

int main(void)
{
    static char *const arguments[] = { "/sbin/init", NULL };
    static char *const environment[] = {
        "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin",
        "TERM=linux",
        NULL
    };

    if (ensure_directory("/dev") != 0 ||
        ensure_directory("/dev/pts") != 0 ||
        ensure_directory("/proc") != 0 ||
        ensure_directory("/sys") != 0 ||
        ensure_directory("/run") != 0 ||
        ensure_directory("/run/lock") != 0)
        return 1;

    if (mount_filesystem("devtmpfs", "/dev", "devtmpfs",
                         MS_NOSUID, "mode=0755") != 0 ||
        mount_filesystem("devpts", "/dev/pts", "devpts",
                         MS_NOSUID | MS_NOEXEC, "mode=0620,ptmxmode=0666") != 0 ||
        mount_filesystem("proc", "/proc", "proc",
                         MS_NOSUID | MS_NOEXEC | MS_NODEV, NULL) != 0 ||
        mount_filesystem("sysfs", "/sys", "sysfs",
                         MS_NOSUID | MS_NOEXEC | MS_NODEV, NULL) != 0 ||
        mount_filesystem("tmpfs", "/run", "tmpfs",
                         MS_NOSUID | MS_NODEV, "mode=0755") != 0)
        return 1;

    execve("/sbin/init", arguments, environment);
    fprintf(stderr, "Mintosh init: cannot execute system init: %s\n", strerror(errno));
    return 1;
}

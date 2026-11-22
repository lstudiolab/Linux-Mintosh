#define _GNU_SOURCE
#include <xcu/xcu.h>

#include <errno.h>
#include <fcntl.h>
#include <linux/memfd.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <unistd.h>

int xcu_memory_create(const char *name, size_t size)
{
    if (name == NULL || name[0] == '\0' || size == 0) {
        errno = EINVAL;
        return -1;
    }
#ifdef SYS_memfd_create
    int descriptor = (int)syscall(SYS_memfd_create, name,
                                  MFD_CLOEXEC | MFD_ALLOW_SEALING);
    if (descriptor < 0) return -1;
    off_t object_size = (off_t)size;
    if (object_size <= 0 || (uint64_t)object_size != (uint64_t)size ||
        ftruncate(descriptor, object_size) < 0 ||
        fcntl(descriptor, F_ADD_SEALS, F_SEAL_GROW | F_SEAL_SHRINK | F_SEAL_SEAL) < 0) {
        int saved = errno;
        close(descriptor);
        errno = saved;
        return -1;
    }
    return descriptor;
#else
    (void)size;
    errno = ENOSYS;
    return -1;
#endif
}

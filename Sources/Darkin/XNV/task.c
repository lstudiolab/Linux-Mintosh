#define _GNU_SOURCE
#include "include/xnv/xnv.h"

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

int xnv_task_open(pid_t pid, xnv_task *task)
{
    if (pid <= 0 || task == NULL) { errno = EINVAL; return -1; }
    task->pid = -1;
    task->pidfd = -1;
#ifdef SYS_pidfd_open
    int descriptor = (int)syscall(SYS_pidfd_open, pid, 0U);
    if (descriptor < 0) return -1;
    task->pid = pid;
    task->pidfd = descriptor;
    return 0;
#else
    (void)pid;
    errno = ENOSYS;
    return -1;
#endif
}

int xnv_task_signal(const xnv_task *task, int signal_number)
{
    if (task == NULL || task->pidfd < 0 || signal_number < 0) {
        errno = EINVAL;
        return -1;
    }
#ifdef SYS_pidfd_send_signal
    return (int)syscall(SYS_pidfd_send_signal, task->pidfd, signal_number, NULL, 0U);
#else
    errno = ENOSYS;
    return -1;
#endif
}

void xnv_task_close(xnv_task *task)
{
    if (task == NULL) return;
    if (task->pidfd >= 0) close(task->pidfd);
    task->pidfd = -1;
    task->pid = -1;
}

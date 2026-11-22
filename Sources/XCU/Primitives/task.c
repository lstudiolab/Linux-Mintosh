#define _GNU_SOURCE
#include <xcu/xcu.h>

#include <errno.h>
#include <signal.h>
#include <sys/syscall.h>
#include <unistd.h>

int xcu_task_open(pid_t pid, xcu_task *task)
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
    errno = ENOSYS;
    return -1;
#endif
}

int xcu_task_signal(const xcu_task *task, int signal_number)
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

void xcu_task_close(xcu_task *task)
{
    if (task == NULL) return;
    if (task->pidfd >= 0) close(task->pidfd);
    task->pidfd = -1;
    task->pid = -1;
}

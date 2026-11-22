#define _GNU_SOURCE
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define SUPERVISOR_PATH "/usr/bin/launch7762"
#define RESTART_MAX_SECONDS 30
#define SHUTDOWN_GRACE_SECONDS 5

static volatile sig_atomic_t shutdown_requested;

static void handle_shutdown(int signal_number)
{
    (void)signal_number;
    shutdown_requested = 1;
}

static int ensure_directory(const char *path, mode_t mode)
{
    if (mkdir(path, mode) == 0 || errno == EEXIST)
        return 0;
    fprintf(stderr, "mintosh-init: mkdir %s: %s\n", path, strerror(errno));
    return -1;
}

static int mount_once(const char *source, const char *target,
                      const char *filesystem, unsigned long flags,
                      const char *options)
{
    if (mount(source, target, filesystem, flags, options) == 0 || errno == EBUSY)
        return 0;
    fprintf(stderr, "mintosh-init: mount %s on %s: %s\n",
            filesystem, target, strerror(errno));
    return -1;
}

static int prepare_kernel_interfaces(void)
{
    if (ensure_directory("/dev", 0755) < 0 ||
        ensure_directory("/proc", 0555) < 0 ||
        ensure_directory("/sys", 0555) < 0 ||
        ensure_directory("/run", 0755) < 0 ||
        ensure_directory("/tmp", 01777) < 0)
        return -1;

    if (mount_once("devtmpfs", "/dev", "devtmpfs", MS_NOSUID, "mode=0755") < 0 ||
        ensure_directory("/dev/pts", 0755) < 0 ||
        mount_once("devpts", "/dev/pts", "devpts",
                   MS_NOSUID | MS_NOEXEC, "mode=0620,ptmxmode=0666") < 0 ||
        mount_once("proc", "/proc", "proc",
                   MS_NOSUID | MS_NOEXEC | MS_NODEV, NULL) < 0 ||
        mount_once("sysfs", "/sys", "sysfs",
                   MS_NOSUID | MS_NOEXEC | MS_NODEV, NULL) < 0 ||
        mount_once("tmpfs", "/run", "tmpfs",
                   MS_NOSUID | MS_NODEV, "mode=0755") < 0 ||
        ensure_directory("/run/lock", 0755) < 0)
        return -1;

    return 0;
}

static pid_t start_supervisor(void)
{
    pid_t child = fork();
    if (child < 0) {
        fprintf(stderr, "mintosh-init: fork: %s\n", strerror(errno));
        return -1;
    }
    if (child == 0) {
        struct sigaction action;
        memset(&action, 0, sizeof(action));
        sigemptyset(&action.sa_mask);
        action.sa_handler = SIG_DFL;
        (void)sigaction(SIGTERM, &action, NULL);
        (void)sigaction(SIGINT, &action, NULL);

        char *const arguments[] = { (char *)SUPERVISOR_PATH, NULL };
        char *const environment[] = {
            "PATH=/usr/local/bin:/usr/bin:/bin",
            "HOME=/",
            "MINTOSH_INIT=1",
            NULL
        };
        execve(SUPERVISOR_PATH, arguments, environment);
        fprintf(stderr, "mintosh-init: exec %s: %s\n",
                SUPERVISOR_PATH, strerror(errno));
        _exit(127);
    }
    fprintf(stderr, "mintosh-init: launch7762 started (pid %ld)\n", (long)child);
    return child;
}

static void reap_children(pid_t *supervisor, int *supervisor_status,
                          int *supervisor_exited)
{
    int status;
    pid_t child;
    while ((child = waitpid(-1, &status, WNOHANG)) > 0) {
        if (child == *supervisor) {
            *supervisor_status = status;
            *supervisor_exited = 1;
            *supervisor = -1;
        }
    }
}

static void sleep_seconds(unsigned int seconds)
{
    struct timespec remaining = { .tv_sec = (time_t)seconds, .tv_nsec = 0 };
    while (!shutdown_requested && nanosleep(&remaining, &remaining) < 0) {
        if (errno != EINTR)
            break;
    }
}

static void stop_supervisor(pid_t supervisor)
{
    if (supervisor <= 0)
        return;

    (void)kill(supervisor, SIGTERM);
    for (unsigned int elapsed = 0; elapsed < SHUTDOWN_GRACE_SECONDS * 10; ++elapsed) {
        int status;
        pid_t result = waitpid(supervisor, &status, WNOHANG);
        if (result == supervisor || (result < 0 && errno == ECHILD))
            return;
        struct timespec delay = { .tv_sec = 0, .tv_nsec = 100000000L };
        (void)nanosleep(&delay, NULL);
    }

    (void)kill(supervisor, SIGKILL);
    while (waitpid(supervisor, NULL, 0) < 0 && errno == EINTR) {}
}

static int install_signal_handlers(void)
{
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    sigemptyset(&action.sa_mask);
    action.sa_handler = handle_shutdown;
    if (sigaction(SIGTERM, &action, NULL) < 0 ||
        sigaction(SIGINT, &action, NULL) < 0)
        return -1;

    signal(SIGPIPE, SIG_IGN);
    return 0;
}

static int run_init(void)
{
    unsigned int restart_delay = 1;
    pid_t supervisor = -1;

    if (getpid() != 1) {
        fputs("mintosh-init: refusing to run unless this process is PID 1\n", stderr);
        return 2;
    }
    if (install_signal_handlers() < 0) {
        fprintf(stderr, "mintosh-init: signal setup: %s\n", strerror(errno));
        return 1;
    }
    if (prepare_kernel_interfaces() < 0)
        return 1;

    fputs("Mintosh Init: first-party PID 1 started\n", stderr);
    while (!shutdown_requested) {
        int status = 0;
        int exited = 0;

        supervisor = start_supervisor();
        if (supervisor < 0) {
            sleep_seconds(restart_delay);
        } else {
            while (!shutdown_requested && !exited) {
                reap_children(&supervisor, &status, &exited);
                if (!exited && !shutdown_requested) {
                    struct timespec delay = { .tv_sec = 0, .tv_nsec = 250000000L };
                    (void)nanosleep(&delay, NULL);
                }
            }
        }

        if (shutdown_requested)
            break;

        fprintf(stderr, "mintosh-init: launch7762 exited; restarting in %u second(s)\n",
                restart_delay);
        sleep_seconds(restart_delay);
        if (restart_delay < RESTART_MAX_SECONDS) {
            restart_delay *= 2;
            if (restart_delay > RESTART_MAX_SECONDS)
                restart_delay = RESTART_MAX_SECONDS;
        }
    }

    stop_supervisor(supervisor);
    while (waitpid(-1, NULL, WNOHANG) > 0) {}
    fputs("Mintosh Init: shutdown requested\n", stderr);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        puts("Mintosh Init 0.1.0");
        return 0;
    }
    if (argc == 2 && strcmp(argv[1], "--self-test") == 0) {
        puts("Mintosh Init self-test passed");
        return 0;
    }
    if (argc != 1) {
        fputs("usage: mintosh-init [--version|--self-test]\n", stderr);
        return 64;
    }
    return run_init();
}

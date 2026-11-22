#define _GNU_SOURCE
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define SUPERVISOR "/usr/bin/launch7762"
#define RESTART_MAX_SECONDS 60

static volatile sig_atomic_t stopping;
static volatile sig_atomic_t child_changed;

static void handle_term(int signal_number) { (void)signal_number; stopping = 1; }
static void handle_child(int signal_number) { (void)signal_number; child_changed = 1; }

static int ensure_directory(const char *path, mode_t mode) {
    if (mkdir(path, mode) == 0 || errno == EEXIST) {
        struct stat metadata;
        return lstat(path, &metadata) == 0 && S_ISDIR(metadata.st_mode) ? 0 : -1;
    }
    return -1;
}

static int mount_once(const char *source, const char *target, const char *filesystem,
                      unsigned long flags, const char *options) {
    if (mount(source, target, filesystem, flags, options) == 0 || errno == EBUSY) return 0;
    fprintf(stderr, "Mintosh init: mount %s on %s: %s\n", filesystem, target, strerror(errno));
    return -1;
}

static int prepare_kernel_interfaces(void) {
    if (ensure_directory("/dev", 0755) < 0 || ensure_directory("/dev/pts", 0755) < 0 ||
        ensure_directory("/proc", 0555) < 0 || ensure_directory("/sys", 0555) < 0 ||
        ensure_directory("/run", 0755) < 0 || ensure_directory("/run/lock", 0755) < 0)
        return -1;
    return mount_once("devtmpfs", "/dev", "devtmpfs", MS_NOSUID, "mode=0755") ||
           mount_once("devpts", "/dev/pts", "devpts", MS_NOSUID | MS_NOEXEC,
                      "mode=0620,ptmxmode=0666") ||
           mount_once("proc", "/proc", "proc", MS_NOSUID | MS_NOEXEC | MS_NODEV, NULL) ||
           mount_once("sysfs", "/sys", "sysfs", MS_NOSUID | MS_NOEXEC | MS_NODEV, NULL) ||
           mount_once("tmpfs", "/run", "tmpfs", MS_NOSUID | MS_NODEV, "mode=0755") ? -1 : 0;
}

static pid_t start_supervisor(void) {
    pid_t child = fork();
    if (child != 0) return child;
    char *const arguments[] = { (char *)SUPERVISOR, NULL };
    char *const environment[] = {
        "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin",
        "HOME=/root", "MINTOSH_SERVICE_DIR=/etc/mintosh/services",
        "MINTOSH_RUN_DIR=/run/mintosh", NULL
    };
    execve(SUPERVISOR, arguments, environment);
    dprintf(STDERR_FILENO, "Mintosh init: cannot start launch7762: %s\n", strerror(errno));
    _exit(127);
}

static void pause_seconds(unsigned seconds) {
    struct timespec remaining = { .tv_sec = (time_t)seconds, .tv_nsec = 0 };
    while (!stopping && nanosleep(&remaining, &remaining) < 0 && errno == EINTR) {}
}

static int run_pid_one(void) {
    struct sigaction action = {0};
    action.sa_handler = handle_term;
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGTERM, &action, NULL) < 0 || sigaction(SIGINT, &action, NULL) < 0) return 1;
    action.sa_handler = handle_child;
    if (sigaction(SIGCHLD, &action, NULL) < 0) return 1;
    signal(SIGPIPE, SIG_IGN);

    if (prepare_kernel_interfaces() < 0) {
        fputs("Mintosh init: cannot prepare kernel interfaces; refusing partial startup\n", stderr);
        return 1;
    }
    (void)mkdir("/run/mintosh", 0755);
    (void)chdir("/");
    (void)umask(022);
    puts("Mintosh native init: starting service foundation");

    unsigned delay = 1;
    pid_t supervisor = start_supervisor();
    if (supervisor < 0) { perror("Mintosh init: fork launch7762"); return 1; }

    while (!stopping) {
        int status;
        pid_t child;
        child_changed = 0;
        while ((child = waitpid(-1, &status, WNOHANG)) > 0) {
            if (child == supervisor) {
                supervisor = 0;
                fputs("Mintosh init: launch7762 exited; restart scheduled\n", stderr);
            }
        }
        if (supervisor == 0 && !stopping) {
            pause_seconds(delay);
            if (stopping) break;
            supervisor = start_supervisor();
            if (supervisor < 0) { perror("Mintosh init: restart launch7762"); supervisor = 0; }
            if (delay < RESTART_MAX_SECONDS) {
                delay *= 2;
                if (delay > RESTART_MAX_SECONDS) delay = RESTART_MAX_SECONDS;
            }
        } else if (supervisor > 0 && !child_changed) pause_seconds(1);
    }

    if (supervisor > 0) {
        (void)kill(supervisor, SIGTERM);
        for (unsigned elapsed = 0; elapsed < 10; ++elapsed) {
            int status;
            pid_t result = waitpid(supervisor, &status, WNOHANG);
            if (result == supervisor || (result < 0 && errno == ECHILD)) { supervisor = 0; break; }
            pause_seconds(1);
        }
        if (supervisor > 0) {
            (void)kill(supervisor, SIGKILL);
            while (waitpid(supervisor, NULL, 0) < 0 && errno == EINTR) {}
        }
    }
    while (waitpid(-1, NULL, WNOHANG) > 0) {}
    sync();
    return 0;
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        puts("Mintosh Init 0.1.0"); return 0;
    }
    if (argc == 2 && strcmp(argv[1], "--self-test") == 0) {
        puts("Mintosh Init argument and build self-test passed"); return 0;
    }
    if (argc != 1) { fputs("Usage: mintosh-init [--version|--self-test]\n", stderr); return 64; }
    if (getpid() != 1) { fputs("Mintosh init: must be executed as PID 1\n", stderr); return 2; }
    return run_pid_one();
}

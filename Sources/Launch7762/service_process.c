#define _GNU_SOURCE
#include "service_manager.h"
#include "libsys.h"
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

#define SERVICE_STOP_GRACE_SECONDS 3
#define SERVICE_RESTART_MAX_SECONDS 60
#define SERVICE_ARGUMENT_LIMIT 32
#define SERVICE_RESTART_LIMIT 10
#define SERVICE_STABLE_SECONDS 300

static struct timespec monotonic_now(void) {
    struct timespec now = {0};
    (void)clock_gettime(CLOCK_MONOTONIC, &now);
    return now;
}

static struct timespec add_seconds(struct timespec value, time_t seconds) {
    value.tv_sec += seconds;
    return value;
}

static int time_reached(struct timespec now, struct timespec deadline) {
    return now.tv_sec > deadline.tv_sec ||
           (now.tv_sec == deadline.tv_sec && now.tv_nsec >= deadline.tv_nsec);
}

/* ExecStart is a plain whitespace-separated argument vector, never a shell program. */
static int is_script_interpreter(const char *path) {
    const char *name = strrchr(path, '/');
    name = name ? name + 1 : path;
    static const char *const interpreters[] = {
        "sh", "bash", "dash", "zsh", "ksh", "fish", "csh", "tcsh",
        "python", "python2", "python3", "perl", "ruby", "node", "php",
        "lua", "luajit", "tclsh", "wish", "Rscript", "busybox", "env",
        NULL
    };
    for (size_t index = 0; interpreters[index]; ++index)
        if (strcmp(name, interpreters[index]) == 0) return 1;
    return 0;
}

static int trusted_executable(const char *path) {
    if (!path || path[0] != '/' || path[1] == '\0' ||
        strstr(path, "//") || is_script_interpreter(path))
        return 0;

    char copy[MINTOSH_SERVICE_COMMAND_MAX];
    if (strlen(path) >= sizeof copy) return 0;
    strcpy(copy, path);

    int directory = open("/", O_PATH | O_DIRECTORY | O_CLOEXEC);
    if (directory < 0) return 0;
    char *cursor = copy + 1;
    char *component = cursor;
    int valid = 1;

    for (;;) {
        char *separator = strchr(cursor, '/');
        if (separator) *separator = '\0';
        if (!component[0] || strcmp(component, ".") == 0 ||
            strcmp(component, "..") == 0) {
            valid = 0;
            break;
        }

        if (separator) {
            int next = openat(directory, component,
                              O_PATH | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
            struct stat metadata;
            if (next < 0 || fstat(next, &metadata) < 0 ||
                !S_ISDIR(metadata.st_mode) ||
                (metadata.st_uid != 0 && metadata.st_uid != geteuid()) ||
                (metadata.st_mode & 0022) != 0) {
                if (next >= 0) close(next);
                valid = 0;
                break;
            }
            close(directory);
            directory = next;
            cursor = separator + 1;
            component = cursor;
            if (!*cursor) { valid = 0; break; }
        } else {
            int executable = openat(directory, component,
                                    O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
            struct stat metadata;
            if (executable < 0 || fstat(executable, &metadata) < 0 ||
                !S_ISREG(metadata.st_mode) ||
                (metadata.st_uid != 0 && metadata.st_uid != geteuid()) ||
                (metadata.st_mode & 0022) != 0 ||
                (metadata.st_mode & 0111) == 0)
                valid = 0;
            if (executable >= 0) close(executable);
            break;
        }
    }

    close(directory);
    return valid;
}

static int parse_arguments(char *command, char **arguments, size_t capacity) {
    size_t count = 0;
    char *cursor = command;
    while (*cursor) {
        while (*cursor && isspace((unsigned char)*cursor)) ++cursor;
        if (!*cursor) break;
        if (count + 1 >= capacity) return -1;
        arguments[count++] = cursor;
        while (*cursor && !isspace((unsigned char)*cursor)) ++cursor;
        if (*cursor) *cursor++ = '\0';
    }
    arguments[count] = NULL;
    if (!count || arguments[0][0] != '/') return -1;
    for (size_t index = 0; index < count; ++index)
        for (const unsigned char *p = (const unsigned char *)arguments[index]; *p; ++p)
            if (*p < 0x20 || *p == 0x7f) return -1;
    return (int)count;
}

int ms_service_start(MintoshService *service) {
    if (!service || !service->command[0]) { errno = EINVAL; return -1; }
    if (service->process_id > 0) return 0;

    char validated_command[MINTOSH_SERVICE_COMMAND_MAX];
    char *validated_arguments[SERVICE_ARGUMENT_LIMIT];
    memcpy(validated_command, service->command, sizeof validated_command);
    validated_command[sizeof validated_command - 1] = '\0';
    if (parse_arguments(validated_command, validated_arguments,
                        SERVICE_ARGUMENT_LIMIT) < 0 ||
        !trusted_executable(validated_arguments[0])) {
        errno = EPERM;
        ms_log("launch7762", "error",
               "refused service executable outside trusted root-owned paths");
        return -1;
    }

    pid_t child = fork();
    if (child < 0) {
        ms_log("launch7762", "error", "unable to fork service process");
        return -1;
    }
    if (child == 0) {
        char command[MINTOSH_SERVICE_COMMAND_MAX];
        char *arguments[SERVICE_ARGUMENT_LIMIT];
        memcpy(command, service->command, sizeof command);
        command[sizeof command - 1] = '\0';
        if (setsid() < 0 || parse_arguments(command, arguments,
                                             SERVICE_ARGUMENT_LIMIT) < 0)
            _exit(126);
        (void)umask(027);
        if (chdir("/") < 0) _exit(126);
        if (close_range(3, UINT_MAX, 0) < 0) {
            long maximum = sysconf(_SC_OPEN_MAX);
            if (maximum < 0) maximum = 65536;
            for (int descriptor = 3; descriptor < maximum; ++descriptor)
                (void)close(descriptor);
        }
        char *environment[] = {
            "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin",
            "LANG=C.UTF-8",
            NULL
        };
        char **child_environment = geteuid() == 0 ? environment : environ;
        execve(arguments[0], arguments, child_environment);
        _exit(errno == ENOENT ? 127 : 126);
    }
    service->process_id = child;
    service->restart_pending = 0;
    service->kill_sent = 0;
    service->stop_deadline = (struct timespec){0};
    service->started_at = monotonic_now();
    char message[160];
    snprintf(message, sizeof message, "started %s with pid %ld",
             service->name, (long)child);
    ms_log("launch7762", "info", message);
    return 0;
}

void ms_service_stop(MintoshService *service) {
    if (!service) return;
    service->enabled = 0;
    service->restart_pending = 0;
    service->restart_count = 0;
    if (service->process_id > 0) {
        (void)kill(-service->process_id, SIGTERM);
        (void)kill(service->process_id, SIGTERM);
        service->stop_deadline = add_seconds(monotonic_now(), SERVICE_STOP_GRACE_SECONDS);
        service->kill_sent = 0;
    }
}

static void schedule_restart(MintoshService *service, struct timespec now) {
    unsigned int shift = service->restart_count > 6 ? 6 : service->restart_count;
    time_t delay = (time_t)1 << shift;
    if (delay > SERVICE_RESTART_MAX_SECONDS) delay = SERVICE_RESTART_MAX_SECONDS;
    ++service->restart_count;
    service->restart_pending = 1;
    service->restart_at = add_seconds(now, delay);
}

void ms_services_reap(MintoshServiceManager *manager, int shutting_down) {
    struct timespec now = monotonic_now();
    int status;
    pid_t child;
    while ((child = waitpid(-1, &status, WNOHANG)) > 0) {
        for (size_t index = 0; index < manager->count; ++index) {
            MintoshService *service = &manager->services[index];
            if (service->process_id != child) continue;
            service->process_id = 0;
            service->stop_deadline = (struct timespec){0};
            service->kill_sent = 0;
            time_t runtime = now.tv_sec - service->started_at.tv_sec;
            service->started_at = (struct timespec){0};
            if (runtime >= SERVICE_STABLE_SECONDS) service->restart_count = 0;
            char message[160];
            snprintf(message, sizeof message, "%s exited (wait status %d)",
                     service->name, status);
            ms_log("launch7762", "warning", message);

            int failed = !WIFEXITED(status) || WEXITSTATUS(status) != 0;
            int should_restart = service->enabled && !shutting_down &&
                (service->restart_policy == MINTOSH_RESTART_ALWAYS ||
                 (service->restart_policy == MINTOSH_RESTART_ON_FAILURE && failed));
            if (should_restart && service->restart_count < SERVICE_RESTART_LIMIT) {
                schedule_restart(service, now);
            } else if (should_restart) {
                service->enabled = 0;
                ms_log("launch7762", "error",
                       "service disabled after reaching its restart limit");
            }
            break;
        }
    }

    for (size_t index = 0; index < manager->count; ++index) {
        MintoshService *service = &manager->services[index];
        if (service->process_id > 0 && service->stop_deadline.tv_sec != 0 &&
            !service->kill_sent && time_reached(now, service->stop_deadline)) {
            (void)kill(-service->process_id, SIGKILL);
            (void)kill(service->process_id, SIGKILL);
            service->kill_sent = 1;
            ms_log("launch7762", "warning", "service exceeded stop grace period; sent SIGKILL");
        }
        if (service->process_id == 0 && service->restart_pending &&
            service->enabled && !shutting_down && time_reached(now, service->restart_at)) {
            service->restart_pending = 0;
            (void)ms_service_start(service);
        }
    }
}

void ms_services_stop_all(MintoshServiceManager *manager) {
    for (size_t index = 0; index < manager->count; ++index)
        ms_service_stop(&manager->services[index]);
}

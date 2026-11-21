#define _GNU_SOURCE
#include "task_policy.h"
#include <dirent.h>
#include <errno.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct {
    pid_t pid;
    pid_t parent;
    uid_t owner;
    int nice_value;
    int scheduler;
    char state;
    char name[256];
} GVDTask;

static int numeric_name(const char *value) {
    if (!value || !value[0]) return 0;
    for (const unsigned char *p = (const unsigned char *)value; *p; ++p)
        if (*p < '0' || *p > '9') return 0;
    return 1;
}

static int read_task(pid_t pid, GVDTask *task) {
    char path[128], buffer[4096];
    snprintf(path, sizeof path, "/proc/%ld/stat", (long)pid);
    FILE *file = fopen(path, "r");
    if (!file) return -1;
    if (!fgets(buffer, sizeof buffer, file)) { fclose(file); return -1; }
    fclose(file);

    char *closing = strrchr(buffer, ')');
    if (!closing || closing[1] != ' ') return -1;
    char *save = NULL;
    char *token = strtok_r(closing + 2, " ", &save);
    unsigned int field = 3;
    int got_parent = 0, got_nice = 0;
    while (token) {
        if (field == 3) task->state = token[0];
        if (field == 4) {
            char *end = NULL;
            long value = strtol(token, &end, 10);
            if (!end || *end || value < 0 || value > INT_MAX) return -1;
            task->parent = (pid_t)value;
            got_parent = 1;
        }
        if (field == 19) {
            char *end = NULL;
            long value = strtol(token, &end, 10);
            if (!end || *end || value < -20 || value > 19) return -1;
            task->nice_value = (int)value;
            got_nice = 1;
            break;
        }
        token = strtok_r(NULL, " ", &save);
        ++field;
    }
    if (!got_parent || !got_nice) return -1;

    snprintf(path, sizeof path, "/proc/%ld/comm", (long)pid);
    file = fopen(path, "r");
    if (!file) return -1;
    if (!fgets(task->name, sizeof task->name, file)) task->name[0] = '\0';
    fclose(file);
    task->name[strcspn(task->name, "\r\n")] = '\0';

    struct stat metadata;
    snprintf(path, sizeof path, "/proc/%ld", (long)pid);
    if (stat(path, &metadata) < 0) return -1;
    task->owner = metadata.st_uid;
    task->scheduler = sched_getscheduler(pid);
    if (task->scheduler < 0) task->scheduler = SCHED_OTHER;
    task->pid = pid;
    return 0;
}

static const char *policy_name(int policy) {
    if (policy == SCHED_BATCH) return "batch";
    if (policy == SCHED_IDLE) return "idle";
    if (policy == SCHED_FIFO) return "fifo";
    if (policy == SCHED_RR) return "round-robin";
    return "normal";
}

static void json_string(const char *value) {
    putchar('"');
    for (const unsigned char *p = (const unsigned char *)value; *p; ++p) {
        if (*p == '"' || *p == '\\') { putchar('\\'); putchar(*p); }
        else if (*p < 0x20) printf("\\u%04x", *p);
        else putchar(*p);
    }
    putchar('"');
}

static int list_tasks(int json) {
    DIR *proc = opendir("/proc");
    if (!proc) { perror("gvdctl: /proc"); return 1; }
    if (json) puts("[");
    else puts("PID      PPID     UID      STATE NICE POLICY       COMMAND");
    struct dirent *entry;
    int first = 1;
    while ((entry = readdir(proc)) != NULL) {
        if (!numeric_name(entry->d_name)) continue;
        char *end = NULL;
        long number = strtol(entry->d_name, &end, 10);
        if (!end || *end || number <= 0 || number > INT_MAX) continue;
        GVDTask task = {0};
        if (read_task((pid_t)number, &task) < 0) continue;
        if (json) {
            if (!first) puts(",");
            printf("{\"pid\":%ld,\"parent\":%ld,\"uid\":%lu,\"state\":",
                   (long)task.pid, (long)task.parent, (unsigned long)task.owner);
            char state[2] = {task.state, '\0'};
            json_string(state);
            printf(",\"nice\":%d,\"policy\":", task.nice_value);
            json_string(policy_name(task.scheduler));
            fputs(",\"command\":", stdout);
            json_string(task.name);
            putchar('}');
            first = 0;
        } else {
            printf("%-8ld %-8ld %-8lu %-5c %-4d %-12s %s\n",
                   (long)task.pid, (long)task.parent, (unsigned long)task.owner,
                   task.state, task.nice_value, policy_name(task.scheduler), task.name);
        }
    }
    if (json) puts("\n]");
    closedir(proc);
    return 0;
}

static int change_priority(const char *pid_text, const char *nice_text) {
    pid_t pid;
    int nice_value;
    if (gvd_parse_pid(pid_text, &pid) < 0 ||
        gvd_parse_nice(nice_text, &nice_value) < 0) {
        fputs("usage: gvdctl priority PID NICE(-20..19)\n", stderr);
        return 64;
    }
    if (setpriority(PRIO_PROCESS, (id_t)pid, nice_value) < 0) {
        perror("gvdctl: setpriority");
        return 1;
    }
    printf("Updated PID %ld nice value to %d.\n", (long)pid, nice_value);
    return 0;
}

static int change_policy(const char *pid_text, const char *policy_text) {
    pid_t pid;
    int policy;
    if (gvd_parse_pid(pid_text, &pid) < 0 ||
        gvd_parse_policy(policy_text, &policy) < 0) {
        fputs("usage: gvdctl policy PID normal|batch|idle\n", stderr);
        return 64;
    }
    struct sched_param parameters = {0};
    if (sched_setscheduler(pid, policy, &parameters) < 0) {
        perror("gvdctl: sched_setscheduler");
        return 1;
    }
    printf("Updated PID %ld scheduling policy to %s.\n",
           (long)pid, policy_name(policy));
    return 0;
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        puts("GVD Task Manager 0.1.0");
        return 0;
    }
    if (argc == 2 && strcmp(argv[1], "list") == 0) return list_tasks(0);
    if (argc == 3 && strcmp(argv[1], "list") == 0 &&
        strcmp(argv[2], "--json") == 0) return list_tasks(1);
    if (argc == 4 && strcmp(argv[1], "priority") == 0)
        return change_priority(argv[2], argv[3]);
    if (argc == 4 && strcmp(argv[1], "policy") == 0)
        return change_policy(argv[2], argv[3]);
    fputs("usage: gvdctl list [--json] | priority PID NICE | policy PID normal|batch|idle\n", stderr);
    return 64;
}

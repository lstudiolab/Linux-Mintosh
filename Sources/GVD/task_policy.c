#include "task_policy.h"
#include <errno.h>
#include <limits.h>
#include <sched.h>
#include <stdlib.h>
#include <string.h>

int gvd_parse_pid(const char *text, pid_t *pid) {
    if (!text || !text[0] || !pid) return -1;
    errno = 0;
    char *end = NULL;
    long value = strtol(text, &end, 10);
    if (errno || !end || *end || value <= 1 || value > INT_MAX) return -1;
    *pid = (pid_t)value;
    return 0;
}

int gvd_parse_nice(const char *text, int *value) {
    if (!text || !text[0] || !value) return -1;
    errno = 0;
    char *end = NULL;
    long parsed = strtol(text, &end, 10);
    if (errno || !end || *end || parsed < -20 || parsed > 19) return -1;
    *value = (int)parsed;
    return 0;
}

int gvd_parse_policy(const char *text, int *policy) {
    if (!text || !policy) return -1;
    if (strcmp(text, "normal") == 0) *policy = SCHED_OTHER;
    else if (strcmp(text, "batch") == 0) *policy = SCHED_BATCH;
    else if (strcmp(text, "idle") == 0) *policy = SCHED_IDLE;
    else return -1;
    return 0;
}

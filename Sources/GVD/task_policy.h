#ifndef MINTOSH_GVD_TASK_POLICY_H
#define MINTOSH_GVD_TASK_POLICY_H
#include <sys/types.h>
int gvd_parse_pid(const char *text, pid_t *pid);
int gvd_parse_nice(const char *text, int *value);
int gvd_parse_policy(const char *text, int *policy);
#endif

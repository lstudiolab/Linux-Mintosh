#ifndef MINTOSH_IOPS_POWER_CONTROL_H
#define MINTOSH_IOPS_POWER_CONTROL_H
int iops_valid_token(const char *value);
int iops_choice_available(const char *choice, const char *available);
int iops_parse_cpu_id(const char *text, unsigned int *cpu);
#endif

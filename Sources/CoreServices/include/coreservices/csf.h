#ifndef MINTOSH_CSF_H
#define MINTOSH_CSF_H

#include <stddef.h>

#define CSF_VERSION_MAX 32

typedef struct {
    int darl_running;
    int launch_manager_running;
    char mintosh_version[CSF_VERSION_MAX];
    char darl_version[CSF_VERSION_MAX];
} CSFSystemStatus;

int csf_request(const char *request, char *response, size_t capacity);
int csf_system_ping(void);
int csf_system_version(char *response, size_t capacity);
int csf_system_status(CSFSystemStatus *status);
int csf_service_name_is_valid(const char *name);
int csf_service_list(char *response, size_t capacity);
int csf_service_control(const char *action, const char *name);

#endif

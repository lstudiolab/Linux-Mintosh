#ifndef MINTOSH_SERVICE_MANAGER_H
#define MINTOSH_SERVICE_MANAGER_H
#include <stddef.h>
#include <time.h>
#include <sys/types.h>

#define MINTOSH_SERVICE_LIMIT 64
#define MINTOSH_SERVICE_NAME_MAX 64
#define MINTOSH_SERVICE_COMMAND_MAX 512

typedef enum {
    MINTOSH_RESTART_NEVER = 0,
    MINTOSH_RESTART_ON_FAILURE = 1,
    MINTOSH_RESTART_ALWAYS = 2
} MintoshRestartPolicy;

typedef struct {
    char name[MINTOSH_SERVICE_NAME_MAX];
    char command[MINTOSH_SERVICE_COMMAND_MAX];
    int enabled;
    MintoshRestartPolicy restart_policy;
    unsigned int start_order;
    gid_t group_id;
    int has_group;
    int restart_pending;
    int kill_sent;
    unsigned int restart_count;
    pid_t process_id;
    struct timespec restart_at;
    struct timespec started_at;
    struct timespec stop_deadline;
} MintoshService;

typedef struct {
    MintoshService services[MINTOSH_SERVICE_LIMIT];
    size_t count;
} MintoshServiceManager;

int ms_services_load(MintoshServiceManager *manager, const char *directory);
MintoshService *ms_services_find(MintoshServiceManager *manager, const char *name);
int ms_service_start(MintoshService *service);
void ms_service_stop(MintoshService *service);
void ms_services_reap(MintoshServiceManager *manager, int shutting_down);
void ms_services_stop_all(MintoshServiceManager *manager);
void ms_services_list(const MintoshServiceManager *manager, int descriptor);
void ms_services_control(MintoshServiceManager *manager, int descriptor,
                         const char *request, int privileged);
#endif

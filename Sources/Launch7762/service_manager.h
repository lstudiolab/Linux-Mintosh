#ifndef MINTOSH_SERVICE_MANAGER_H
#define MINTOSH_SERVICE_MANAGER_H
#include <stddef.h>
#include <sys/types.h>

#define MINTOSH_SERVICE_LIMIT 64
#define MINTOSH_SERVICE_NAME_MAX 64
#define MINTOSH_SERVICE_COMMAND_MAX 512

typedef struct {
    char name[MINTOSH_SERVICE_NAME_MAX];
    char command[MINTOSH_SERVICE_COMMAND_MAX];
    int enabled;
    int restart_on_exit;
    pid_t process_id;
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
void ms_services_control(MintoshServiceManager *manager, int descriptor, const char *request);
#endif

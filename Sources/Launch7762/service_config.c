#include "service_manager.h"
#include <dirent.h>
#include <stdio.h>
#include <string.h>

int ms_services_load(MintoshServiceManager *manager, const char *directory) {
    DIR *stream = opendir(directory);
    if (!stream) return -1;
    struct dirent *entry;
    while ((entry = readdir(stream)) && manager->count < MINTOSH_SERVICE_LIMIT) {
        size_t filename_length = strlen(entry->d_name);
        if (filename_length <= 8 || strcmp(entry->d_name + filename_length - 8, ".service") != 0) continue;
        char path[512];
        int path_length = snprintf(path, sizeof path, "%s/%s", directory, entry->d_name);
        if (path_length < 0 || (size_t)path_length >= sizeof path) continue;
        FILE *file = fopen(path, "r");
        if (!file) continue;
        MintoshService service = {0};
        snprintf(service.name, sizeof service.name, "%.*s", (int)(filename_length - 8), entry->d_name);
        char line[768];
        while (fgets(line, sizeof line, file)) {
            char *separator = strchr(line, '=');
            if (!separator) continue;
            *separator++ = '\0';
            line[strcspn(line, "\r\n")] = '\0';
            separator[strcspn(separator, "\r\n")] = '\0';
            if (strcmp(line, "ExecStart") == 0) snprintf(service.command, sizeof service.command, "%s", separator);
            else if (strcmp(line, "Enabled") == 0) service.enabled = strcmp(separator, "yes") == 0;
            else if (strcmp(line, "Restart") == 0) service.restart_on_exit = strcmp(separator, "always") == 0 || strcmp(separator, "on-failure") == 0;
        }
        fclose(file);
        if (service.command[0]) manager->services[manager->count++] = service;
    }
    closedir(stream);
    return 0;
}

MintoshService *ms_services_find(MintoshServiceManager *manager, const char *name) {
    for (size_t index = 0; index < manager->count; ++index)
        if (strcmp(manager->services[index].name, name) == 0) return &manager->services[index];
    return NULL;
}

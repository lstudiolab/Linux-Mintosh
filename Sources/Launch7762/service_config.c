#define _GNU_SOURCE
#include "service_manager.h"
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int valid_service_name(const char *name) {
    if (!name[0]) return 0;
    for (const unsigned char *p = (const unsigned char *)name; *p; ++p)
        if (!isalnum(*p) && *p != '-' && *p != '_') return 0;
    return 1;
}

static int parse_restart_policy(const char *value, MintoshRestartPolicy *policy) {
    if (strcmp(value, "no") == 0 || strcmp(value, "never") == 0) {
        *policy = MINTOSH_RESTART_NEVER;
        return 0;
    }
    if (strcmp(value, "on-failure") == 0) {
        *policy = MINTOSH_RESTART_ON_FAILURE;
        return 0;
    }
    if (strcmp(value, "always") == 0) {
        *policy = MINTOSH_RESTART_ALWAYS;
        return 0;
    }
    return -1;
}

static int trusted_owner(uid_t owner) {
    return owner == 0 || owner == geteuid();
}

static int load_service(MintoshServiceManager *manager, const char *directory,
                        const char *filename) {
    size_t filename_length = strlen(filename);
    if (filename_length <= 8 ||
        strcmp(filename + filename_length - 8, ".service") != 0) return 0;

    char name[MINTOSH_SERVICE_NAME_MAX];
    size_t name_length = filename_length - 8;
    if (name_length >= sizeof name) return -1;
    memcpy(name, filename, name_length);
    name[name_length] = '\0';
    if (!valid_service_name(name) || ms_services_find(manager, name)) return -1;

    char path[512];
    int path_length = snprintf(path, sizeof path, "%s/%s", directory, filename);
    if (path_length < 0 || (size_t)path_length >= sizeof path) return -1;

    int descriptor = open(path, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (descriptor < 0) return -1;
    struct stat metadata;
    if (fstat(descriptor, &metadata) < 0 || !S_ISREG(metadata.st_mode) ||
        (metadata.st_mode & 0022) != 0 || !trusted_owner(metadata.st_uid)) {
        close(descriptor);
        errno = EPERM;
        return -1;
    }

    FILE *file = fdopen(descriptor, "r");
    if (!file) { close(descriptor); return -1; }
    MintoshService service = {0};
    memcpy(service.name, name, name_length + 1);
    service.restart_policy = MINTOSH_RESTART_NEVER;
    int valid = 1;
    char line[768];
    while (fgets(line, sizeof line, file)) {
        if (!strchr(line, '\n') && !feof(file)) { valid = 0; break; }
        char *separator = strchr(line, '=');
        if (!separator) continue;
        *separator++ = '\0';
        line[strcspn(line, "\r\n")] = '\0';
        separator[strcspn(separator, "\r\n")] = '\0';
        if (strcmp(line, "ExecStart") == 0) {
            if (!separator[0] || strlen(separator) >= sizeof service.command) valid = 0;
            else snprintf(service.command, sizeof service.command, "%s", separator);
        } else if (strcmp(line, "Enabled") == 0) {
            if (strcmp(separator, "yes") == 0) service.enabled = 1;
            else if (strcmp(separator, "no") != 0) valid = 0;
        } else if (strcmp(line, "Restart") == 0) {
            if (parse_restart_policy(separator, &service.restart_policy) < 0) valid = 0;
        }
        if (!valid) break;
    }
    if (ferror(file)) valid = 0;
    fclose(file);
    if (!valid || !service.command[0]) return -1;
    if (manager->count >= MINTOSH_SERVICE_LIMIT) return -1;
    manager->services[manager->count++] = service;
    return 1;
}

int ms_services_load(MintoshServiceManager *manager, const char *directory) {
    struct stat metadata;
    if (lstat(directory, &metadata) < 0) return -1;
    if (!S_ISDIR(metadata.st_mode) || (metadata.st_mode & 0022) != 0 ||
        !trusted_owner(metadata.st_uid)) {
        errno = EPERM;
        return -1;
    }
    DIR *stream = opendir(directory);
    if (!stream) return -1;
    struct dirent *entry;
    while ((entry = readdir(stream)) != NULL) {
        int result = load_service(manager, directory, entry->d_name);
        if (result < 0) {
            char message[256];
            snprintf(message, sizeof message, "ignored invalid service definition: %.180s",
                     entry->d_name);
            fprintf(stderr, "launch7762: %s\n", message);
        }
    }
    closedir(stream);
    return 0;
}

MintoshService *ms_services_find(MintoshServiceManager *manager, const char *name) {
    for (size_t index = 0; index < manager->count; ++index)
        if (strcmp(manager->services[index].name, name) == 0) return &manager->services[index];
    return NULL;
}

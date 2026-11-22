#define _GNU_SOURCE
#include "service_manager.h"
#include <ctype.h>
#include <dirent.h>
#include <grp.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int valid_service_name(const char *name) {
    if (!name[0]) return 0;
    for (const unsigned char *p = (const unsigned char *)name; *p; ++p)
        if (!((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') ||
              (*p >= '0' && *p <= '9') || *p == '-' || *p == '_')) return 0;
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

static int parse_start_order(const char *value, unsigned int *order) {
    if (!value[0]) return -1;
    errno = 0;
    char *end = NULL;
    unsigned long parsed = strtoul(value, &end, 10);
    if (errno != 0 || end == value || *end != '\\0' || parsed > 9999UL)
        return -1;
    *order = (unsigned int)parsed;
    return 0;
}

static int compare_services(const void *left, const void *right) {
    const MintoshService *a = left;
    const MintoshService *b = right;
    if (a->start_order < b->start_order) return -1;
    if (a->start_order > b->start_order) return 1;
    return strcmp(a->name, b->name);
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
        metadata.st_size < 0 || metadata.st_size > 65536 ||
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
    service.start_order = 10000U;
    unsigned int seen = 0;
    int valid = 1;
    char line[768];
    while (fgets(line, sizeof line, file)) {
        if (!strchr(line, '\n') && !feof(file)) { valid = 0; break; }
        char *separator = strchr(line, '=');
        if (!separator) continue;
        *separator++ = '\0';
        line[strcspn(line, "\r\n")] = '\0';
        separator[strcspn(separator, "\r\n")] = '\0';
        unsigned int key = 0;
        if (strcmp(line, "ExecStart") == 0) {
            key = 1u;
            if (!separator[0] || separator[0] != '/' ||
                strlen(separator) >= sizeof service.command) valid = 0;
            else snprintf(service.command, sizeof service.command, "%s", separator);
        } else if (strcmp(line, "Enabled") == 0) {
            key = 2u;
            if (strcmp(separator, "yes") == 0) service.enabled = 1;
            else if (strcmp(separator, "no") != 0) valid = 0;
        } else if (strcmp(line, "Restart") == 0) {
            key = 4u;
            if (parse_restart_policy(separator, &service.restart_policy) < 0) valid = 0;
        } else if (strcmp(line, "Group") == 0) {
            key = 8u;
            struct group *group = getgrnam(separator);
            if (!separator[0] || group == NULL) valid = 0;
            else {
                service.group_id = group->gr_gid;
                service.has_group = 1;
            }
        } else if (strcmp(line, "Order") == 0) {
            key = 16u;
            if (parse_start_order(separator, &service.start_order) < 0) valid = 0;
        }
        if (key && (seen & key)) valid = 0;
        seen |= key;
        if (!valid) break;
    }
    if (ferror(file)) valid = 0;
    fclose(file);
    if (!valid || !(seen & 1u) || !service.command[0]) return -1;
    if (manager->count >= MINTOSH_SERVICE_LIMIT) return -1;
    manager->services[manager->count++] = service;
    return 1;
}

int ms_services_load(MintoshServiceManager *manager, const char *directory) {
    if (!manager || !directory || !directory[0]) { errno = EINVAL; return -1; }
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
    qsort(manager->services, manager->count, sizeof(manager->services[0]),
          compare_services);
    return 0;
}

MintoshService *ms_services_find(MintoshServiceManager *manager, const char *name) {
    if (!manager || !name) return NULL;
    for (size_t index = 0; index < manager->count; ++index)
        if (strcmp(manager->services[index].name, name) == 0) return &manager->services[index];
    return NULL;
}

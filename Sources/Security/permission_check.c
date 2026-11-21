#define _GNU_SOURCE
#include "permission_check.h"
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


int security_check_file_group_permissions(const char *path, mode_t expected_mode,
                                            uid_t expected_owner, gid_t expected_group) {
    struct stat metadata;
    if (lstat(path, &metadata) < 0) { perror(path); return 1; }
    if (!S_ISREG(metadata.st_mode) || metadata.st_uid != expected_owner ||
        metadata.st_gid != expected_group ||
        (metadata.st_mode & 07777) != expected_mode) {
        fprintf(stderr, "WARNING: unsafe file %s owner=%lu group=%lu mode=%04o\n",
                path, (unsigned long)metadata.st_uid, (unsigned long)metadata.st_gid,
                metadata.st_mode & 07777);
        return 1;
    }
    printf("OK: %s (uid %lu gid %lu mode %04o)\n", path,
           (unsigned long)expected_owner, (unsigned long)expected_group, expected_mode);
    return 0;
}

int security_check_socket_permissions(const char *path, uid_t expected_owner,
                                      gid_t expected_group, mode_t expected_mode) {
    struct stat metadata;
    if (lstat(path, &metadata) < 0) { perror(path); return 1; }
    if (!S_ISSOCK(metadata.st_mode) || metadata.st_uid != expected_owner ||
        metadata.st_gid != expected_group ||
        (metadata.st_mode & 07777) != expected_mode) {
        fprintf(stderr, "WARNING: unsafe socket %s owner=%lu group=%lu mode=%04o\n",
                path, (unsigned long)metadata.st_uid, (unsigned long)metadata.st_gid,
                metadata.st_mode & 07777);
        return 1;
    }
    printf("OK: %s socket (uid %lu gid %lu mode %04o)\n", path,
           (unsigned long)expected_owner, (unsigned long)expected_group, expected_mode);
    return 0;
}

int security_check_file_permissions(const char *path, mode_t expected_mode,
                                    uid_t expected_owner) {
    struct stat metadata;
    if (lstat(path, &metadata) < 0) {
        perror(path);
        return 1;
    }
    if (!S_ISREG(metadata.st_mode) || metadata.st_uid != expected_owner ||
        (metadata.st_mode & 07777) != expected_mode) {
        fprintf(stderr, "WARNING: %s owner=%lu mode=%04o (expected uid %lu:%04o regular file)\n",
                path, (unsigned long)metadata.st_uid, metadata.st_mode & 07777,
                (unsigned long)expected_owner, expected_mode);
        return 1;
    }
    printf("OK: %s (uid %lu:%04o)\n", path,
           (unsigned long)expected_owner, expected_mode);
    return 0;
}

int security_check_directory_permissions(const char *path, uid_t expected_owner) {
    struct stat metadata;
    if (lstat(path, &metadata) < 0) {
        perror(path);
        return 1;
    }
    if (!S_ISDIR(metadata.st_mode) || metadata.st_uid != expected_owner ||
        (metadata.st_mode & 0022) != 0) {
        fprintf(stderr, "WARNING: unsafe directory %s owner=%lu mode=%04o (expected uid %lu and no group/world write)\n",
                path, (unsigned long)metadata.st_uid, metadata.st_mode & 07777,
                (unsigned long)expected_owner);
        return 1;
    }
    printf("OK: %s directory (uid %lu; no group/world write)\n",
           path, (unsigned long)expected_owner);
    return 0;
}

int security_check_service_directory(const char *path, uid_t expected_owner) {
    int descriptor = open(path, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (descriptor < 0) {
        perror(path);
        return 1;
    }
    struct stat directory_metadata;
    if (fstat(descriptor, &directory_metadata) < 0 ||
        !S_ISDIR(directory_metadata.st_mode) ||
        directory_metadata.st_uid != expected_owner ||
        (directory_metadata.st_mode & 0022) != 0) {
        fprintf(stderr, "WARNING: unsafe service directory: %s\n", path);
        close(descriptor);
        return 1;
    }

    DIR *directory = fdopendir(descriptor);
    if (!directory) {
        close(descriptor);
        return 1;
    }
    int warnings = 0;
    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        size_t length = strlen(entry->d_name);
        if (length <= 8 || strcmp(entry->d_name + length - 8, ".service") != 0) {
            fprintf(stderr, "WARNING: unexpected service directory entry: %s\n",
                    entry->d_name);
            ++warnings;
            continue;
        }
        int file = openat(dirfd(directory), entry->d_name,
                          O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
        struct stat metadata;
        if (file < 0 || fstat(file, &metadata) < 0 ||
            !S_ISREG(metadata.st_mode) || metadata.st_uid != expected_owner ||
            (metadata.st_mode & 0022) != 0 || metadata.st_size < 0 ||
            metadata.st_size > 65536) {
            fprintf(stderr, "WARNING: unsafe service definition: %s\n",
                    entry->d_name);
            ++warnings;
        } else {
            printf("OK: service definition %s\n", entry->d_name);
        }
        if (file >= 0) close(file);
    }
    closedir(directory);
    return warnings;
}

#define _GNU_SOURCE
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

int security_check_file_permissions(const char *path, mode_t expected_mode) {
    struct stat metadata;
    if (lstat(path, &metadata) < 0) {
        perror(path);
        return 1;
    }
    if (!S_ISREG(metadata.st_mode) || metadata.st_uid != 0 ||
        (metadata.st_mode & 07777) != expected_mode) {
        fprintf(stderr, "WARNING: %s owner=%lu mode=%04o (expected root:%04o regular file)\n",
                path, (unsigned long)metadata.st_uid,
                metadata.st_mode & 07777, expected_mode);
        return 1;
    }
    printf("OK: %s (root:%04o)\n", path, expected_mode);
    return 0;
}

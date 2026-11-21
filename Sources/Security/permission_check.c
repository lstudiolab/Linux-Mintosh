#include <stdio.h>
#include <sys/stat.h>
int security_check_file_permissions(const char *path) {
    struct stat metadata;
    if (stat(path, &metadata) < 0) { perror(path); return 1; }
    if (metadata.st_mode & 0022) {
        fprintf(stderr, "WARNING: %s is writable by group/other (%04o)\n", path, metadata.st_mode & 07777);
        return 1;
    }
    printf("OK: %s (%04o)\n", path, metadata.st_mode & 07777);
    return 0;
}

#define _GNU_SOURCE
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

static int read_value(const char *path, char *buffer, size_t capacity) {
    FILE *file = fopen(path, "r");
    if (!file) return -1;
    if (!fgets(buffer, (int)capacity, file)) { fclose(file); return -1; }
    fclose(file);
    buffer[strcspn(buffer, "\r\n")] = '\0';
    return 0;
}

static void json_string(const char *value) {
    putchar('"');
    for (const unsigned char *cursor = (const unsigned char *)value; *cursor; ++cursor) {
        if (*cursor == '"' || *cursor == '\\\\') { putchar('\\\\'); putchar(*cursor); }
        else if (*cursor < 0x20) printf("\\\\u%04x", *cursor);
        else putchar(*cursor);
    }
    putchar('"');
}

static int list_disks(int json) {
    DIR *directory = opendir("/sys/class/block");
    if (!directory) { perror("dmctl: block devices"); return 1; }
    if (json) putchar('[');
    else puts("DEVICE\tSIZE_BYTES\tREMOVABLE\tREAD_ONLY\tROTATIONAL\tMODEL");
    int first = 1;
    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        char path[512], sectors[64] = "0", removable[16] = "0", read_only[16] = "0";
        char rotational[16] = "?", model[128] = "", partition[32] = "";
        snprintf(path, sizeof path, "/sys/class/block/%s/size", entry->d_name);
        (void)read_value(path, sectors, sizeof sectors);
        snprintf(path, sizeof path, "/sys/class/block/%s/removable", entry->d_name);
        (void)read_value(path, removable, sizeof removable);
        snprintf(path, sizeof path, "/sys/class/block/%s/ro", entry->d_name);
        (void)read_value(path, read_only, sizeof read_only);
        snprintf(path, sizeof path, "/sys/class/block/%s/queue/rotational", entry->d_name);
        (void)read_value(path, rotational, sizeof rotational);
        snprintf(path, sizeof path, "/sys/class/block/%s/device/model", entry->d_name);
        (void)read_value(path, model, sizeof model);
        snprintf(path, sizeof path, "/sys/class/block/%s/partition", entry->d_name);
        int is_partition = read_value(path, partition, sizeof partition) == 0;
        char *end = NULL;
        unsigned long long count = strtoull(sectors, &end, 10);
        unsigned long long bytes = end && !*end ? count * 512ULL : 0;
        if (json) {
            if (!first) putchar(',');
            fputs("{\"name\":", stdout); json_string(entry->d_name);
            printf(",\"size_bytes\":%llu,\"removable\":%s,\"read_only\":%s,\"rotational\":",
                   bytes, strcmp(removable, "1") == 0 ? "true" : "false",
                   strcmp(read_only, "1") == 0 ? "true" : "false");
            if (strcmp(rotational, "0") == 0 || strcmp(rotational, "1") == 0)
                printf("%s", strcmp(rotational, "1") == 0 ? "true" : "false");
            else fputs("null", stdout);
            fputs(",\"partition\":", stdout);
            fputs(is_partition ? "true" : "false", stdout);
            fputs(",\"model\":", stdout); json_string(model);
            putchar('}');
        } else {
            printf("%s\t%llu\t%s\t%s\t%s\t%s%s\n", entry->d_name, bytes,
                   removable, read_only, rotational, model,
                   is_partition ? " (partition)" : "");
        }
        first = 0;
    }
    if (json) puts("]");
    closedir(directory);
    return 0;
}

static int safe_device_path(const char *path) {
    if (!path || strncmp(path, "/dev/", 5) != 0 || strstr(path, "/../") ||
        strcmp(path + (strlen(path) > 3 ? strlen(path) - 3 : 0), "/..") == 0)
        return 0;
    struct stat metadata;
    return stat(path, &metadata) == 0 && S_ISBLK(metadata.st_mode);
}

static int run_mount(int argc, char **argv) {
    if (argc != 4 || !safe_device_path(argv[2])) {
        fputs("usage: dmctl mount /dev/BLOCK_DEVICE /existing/directory\n", stderr);
        return 64;
    }
    char target[4096];
    if (!realpath(argv[3], target)) { perror("dmctl: resolve mount target"); return 1; }
    struct stat metadata;
    if (stat(target, &metadata) < 0 || !S_ISDIR(metadata.st_mode)) {
        fputs("dmctl: mount target must be an existing directory\n", stderr);
        return 64;
    }
    if (geteuid() != 0) {
        fputs("dmctl: mounting filesystems requires root privileges\n", stderr);
        return 2;
    }
    char *const arguments[] = {"/usr/bin/mount", "--", argv[2], target, NULL};
    execv(arguments[0], arguments);
    perror("dmctl: execute mount");
    return 126;
}

static int run_unmount(int argc, char **argv) {
    if (argc != 3 || argv[2][0] != '/') {
        fputs("usage: dmctl unmount /mount/point\n", stderr);
        return 64;
    }
    char target[4096];
    if (!realpath(argv[2], target)) { perror("dmctl: resolve mount point"); return 1; }
    if (geteuid() != 0) {
        fputs("dmctl: unmounting filesystems requires root privileges\n", stderr);
        return 2;
    }
    char *const arguments[] = {"/usr/bin/umount", "--", target, NULL};
    execv(arguments[0], arguments);
    perror("dmctl: execute umount");
    return 126;
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        puts("Mintosh Disk Manager 0.1.0");
        return 0;
    }
    if (argc == 2 && strcmp(argv[1], "list") == 0) return list_disks(0);
    if (argc == 3 && strcmp(argv[1], "list") == 0 &&
        strcmp(argv[2], "--json") == 0) return list_disks(1);
    if (argc >= 2 && strcmp(argv[1], "mount") == 0) return run_mount(argc, argv);
    if (argc >= 2 && strcmp(argv[1], "unmount") == 0) return run_unmount(argc, argv);
    fputs("usage: dmctl list [--json] | mount DEVICE DIRECTORY | unmount DIRECTORY\n", stderr);
    return 64;
}

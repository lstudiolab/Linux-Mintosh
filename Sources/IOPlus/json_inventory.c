#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>

static void print_json_string(const char *value) {
    putchar('"');
    if (value) {
        for (const unsigned char *cursor = (const unsigned char *)value; *cursor; ++cursor) {
            if (*cursor == '"' || *cursor == '\\') {
                putchar('\\');
                putchar(*cursor);
            } else if (*cursor < 0x20) {
                printf("\\u%04x", *cursor);
            } else {
                putchar(*cursor);
            }
        }
    }
    putchar('"');
}

static void print_directory_json(const char *path) {
    DIR *directory = opendir(path);
    putchar('[');
    if (directory) {
        struct dirent *entry;
        int first = 1;
        while ((entry = readdir(directory)) != NULL) {
            if (entry->d_name[0] == '.') continue;
            if (!first) putchar(',');
            print_json_string(entry->d_name);
            first = 0;
        }
        closedir(directory);
    }
    putchar(']');
}

static void read_memory(unsigned long long *total, unsigned long long *available) {
    FILE *file = fopen("/proc/meminfo", "r");
    if (!file) return;
    char line[256];
    while (fgets(line, sizeof line, file)) {
        if (sscanf(line, "MemTotal: %llu kB", total) == 1) continue;
        (void)sscanf(line, "MemAvailable: %llu kB", available);
    }
    fclose(file);
}

void ioplus_print_json(void) {
    struct utsname kernel = {0};
    (void)uname(&kernel);
    unsigned long long total = 0, available = 0;
    read_memory(&total, &available);

    fputs("{\"kernel\":{\"system\":", stdout);
    print_json_string(kernel.sysname);
    fputs(",\"release\":", stdout);
    print_json_string(kernel.release);
    fputs(",\"machine\":", stdout);
    print_json_string(kernel.machine);
    printf("},\"memory_kib\":{\"total\":%llu,\"available\":%llu},\"devices\":{\"block\":",
           total, available);
    print_directory_json("/sys/class/block");
    fputs(",\"network\":", stdout);
    print_directory_json("/sys/class/net");
    fputs(",\"usb\":", stdout);
    print_directory_json("/sys/bus/usb/devices");
    fputs(",\"graphics\":", stdout);
    print_directory_json("/sys/class/drm");
    puts("}}");
}

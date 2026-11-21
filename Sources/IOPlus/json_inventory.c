#define _POSIX_C_SOURCE 200809L
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>

static void print_json_string(const char *value) {
    putchar('"');
    if (value) for (const unsigned char *cursor = (const unsigned char *)value; *cursor; ++cursor) {
        if (*cursor == '"' || *cursor == '\\') { putchar('\\'); putchar(*cursor); }
        else if (*cursor < 0x20) printf("\\u%04x", *cursor);
        else putchar(*cursor);
    }
    putchar('"');
}
static int read_attribute(const char *directory, const char *attribute,
                          char *value, size_t capacity) {
    char path[1024];
    int count = snprintf(path, sizeof path, "%s/%s", directory, attribute);
    if (count < 0 || (size_t)count >= sizeof path) return 0;
    FILE *file = fopen(path, "r");
    if (!file) return 0;
    int ok = fgets(value, (int)capacity, file) != NULL;
    fclose(file);
    if (!ok) return 0;
    value[strcspn(value, "\r\n")] = '\0';
    return 1;
}
static void print_attribute(const char *directory, const char *name) {
    char value[256];
    if (read_attribute(directory, name, value, sizeof value)) {
        putchar(',');
        print_json_string(name);
        putchar(':');
        print_json_string(value);
    }
}
static void print_device_array(const char *path, const char *const *attributes) {
    DIR *directory = opendir(path);
    putchar('[');
    int first = 1;
    if (directory) {
        struct dirent *entry;
        while ((entry = readdir(directory)) != NULL) {
            if (entry->d_name[0] == '.') continue;
            char device_path[1024];
            int count = snprintf(device_path, sizeof device_path, "%s/%s", path, entry->d_name);
            if (count < 0 || (size_t)count >= sizeof device_path) continue;
            if (!first) putchar(',');
            first = 0;
            fputs("{\"name\":", stdout);
            print_json_string(entry->d_name);
            fputs(",\"sysfs_path\":", stdout);
            print_json_string(device_path);
            for (size_t i = 0; attributes[i]; ++i)
                print_attribute(device_path, attributes[i]);
            putchar('}');
        }
        closedir(directory);
    }
    putchar(']');
}
static void print_dmi(void) {
    static const char *const fields[] = {
        "sys_vendor", "product_name", "product_version", "board_name",
        "bios_version", "bios_date", NULL
    };
    fputs("{", stdout);
    int first = 1;
    for (size_t i = 0; fields[i]; ++i) {
        char path[256], value[256];
        snprintf(path, sizeof path, "/sys/class/dmi/id/%s", fields[i]);
        FILE *file = fopen(path, "r");
        if (!file) continue;
        int ok = fgets(value, sizeof value, file) != NULL;
        fclose(file);
        if (!ok) continue;
        value[strcspn(value, "\r\n")] = '\0';
        if (!first) putchar(',');
        first = 0;
        print_json_string(fields[i]);
        putchar(':');
        print_json_string(value);
    }
    putchar('}');
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
static void print_cpu_model(void) {
    FILE *file = fopen("/proc/cpuinfo", "r");
    if (!file) { fputs("null", stdout); return; }
    char line[512], model[256] = {0};
    while (fgets(line, sizeof line, file)) {
        char *separator = strchr(line, ':');
        if (!separator) continue;
        *separator++ = '\0';
        if (strcmp(line, "model name") == 0 || strcmp(line, "Hardware") == 0 ||
            strcmp(line, "Processor") == 0) {
            snprintf(model, sizeof model, "%s", separator);
            model[strcspn(model, "\r\n")] = '\0';
            break;
        }
    }
    fclose(file);
    if (model[0]) print_json_string(model);
    else fputs("null", stdout);
}
void ioplus_print_json(void) {
    static const char *const block[] = {"dev", "size", "removable", "ro", NULL};
    static const char *const network[] = {"address", "operstate", "mtu", "type", NULL};
    static const char *const usb[] = {"idVendor", "idProduct", "manufacturer", "product", "speed", NULL};
    static const char *const graphics[] = {"status", "enabled", "modes", NULL};
    static const char *const power[] = {"type", "status", "capacity", "energy_now", "energy_full", "voltage_now", NULL};
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
    fputs("},\"cpu_model\":", stdout);
    print_cpu_model();
    printf(",\"memory_kib\":{\"total\":%llu,\"available\":%llu},\"firmware\":",
           total, available);
    print_dmi();
    fputs(",\"devices\":{\"block\":", stdout);
    print_device_array("/sys/class/block", block);
    fputs(",\"network\":", stdout);
    print_device_array("/sys/class/net", network);
    fputs(",\"usb\":", stdout);
    print_device_array("/sys/bus/usb/devices", usb);
    fputs(",\"graphics\":", stdout);
    print_device_array("/sys/class/drm", graphics);
    fputs(",\"power\":", stdout);
    print_device_array("/sys/class/power_supply", power);
    puts("}}");
}

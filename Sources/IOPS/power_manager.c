#define _POSIX_C_SOURCE 200809L
#include "power_control.h"
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static int read_value(const char *path, char *buffer, size_t capacity) {
    FILE *file = fopen(path, "r");
    if (!file) return -1;
    if (!fgets(buffer, (int)capacity, file)) {
        fclose(file);
        return -1;
    }
    fclose(file);
    buffer[strcspn(buffer, "\r\n")] = '\0';
    return 0;
}

static int write_value(const char *path, const char *value) {
    int descriptor = open(path, O_WRONLY | O_CLOEXEC);
    if (descriptor < 0) return -1;
    size_t length = strlen(value);
    ssize_t written;
    do { written = write(descriptor, value, length); }
    while (written < 0 && errno == EINTR);
    int saved = errno;
    if (written == (ssize_t)length) {
        do { written = write(descriptor, "\n", 1); }
        while (written < 0 && errno == EINTR);
        if (written == 1) saved = 0;
        else if (written >= 0) saved = EIO;
        else saved = errno;
    } else if (written >= 0) saved = EIO;
    close(descriptor);
    if (saved) { errno = saved; return -1; }
    return 0;
}

static void print_power_supplies(int json) {
    DIR *directory = opendir("/sys/class/power_supply");
    if (!json) puts("Power supplies:");
    if (json) fputs("\"power_supplies\":[", stdout);
    int first = 1;
    if (directory) {
        struct dirent *entry;
        while ((entry = readdir(directory)) != NULL) {
            if (entry->d_name[0] == '.') continue;
            char path[512], type[64] = "", status[64] = "", capacity[64] = "", online[64] = "";
            snprintf(path, sizeof path, "/sys/class/power_supply/%s/type", entry->d_name);
            (void)read_value(path, type, sizeof type);
            snprintf(path, sizeof path, "/sys/class/power_supply/%s/status", entry->d_name);
            (void)read_value(path, status, sizeof status);
            snprintf(path, sizeof path, "/sys/class/power_supply/%s/capacity", entry->d_name);
            (void)read_value(path, capacity, sizeof capacity);
            snprintf(path, sizeof path, "/sys/class/power_supply/%s/online", entry->d_name);
            (void)read_value(path, online, sizeof online);
            if (json) {
                if (!first) putchar(',');
                printf("{\"name\":\"%s\",\"type\":\"%s\",\"status\":\"%s\",\"capacity\":",
                       entry->d_name, type, status);
                if (capacity[0]) printf("%s", capacity); else fputs("null", stdout);
                fputs(",\"online\":", stdout);
                if (online[0]) printf("%s", online); else fputs("null", stdout);
                putchar('}');
            } else {
                printf("  %s: type=%s status=%s capacity=%s online=%s\n",
                       entry->d_name, type[0] ? type : "unknown",
                       status[0] ? status : "unknown",
                       capacity[0] ? capacity : "n/a",
                       online[0] ? online : "n/a");
            }
            first = 0;
        }
        closedir(directory);
    }
    if (json) putchar(']');
    else if (first) puts("  unavailable");
}

static void print_thermal_zones(int json) {
    DIR *directory = opendir("/sys/class/thermal");
    if (!json) puts("Thermal zones:");
    if (json) fputs(",\"thermal_zones\":[", stdout);
    int first = 1;
    if (directory) {
        struct dirent *entry;
        while ((entry = readdir(directory)) != NULL) {
            if (strncmp(entry->d_name, "thermal_zone", 12) != 0) continue;
            char path[512], type[128] = "", temperature[64] = "";
            snprintf(path, sizeof path, "/sys/class/thermal/%s/type", entry->d_name);
            (void)read_value(path, type, sizeof type);
            snprintf(path, sizeof path, "/sys/class/thermal/%s/temp", entry->d_name);
            (void)read_value(path, temperature, sizeof temperature);
            if (json) {
                if (!first) putchar(',');
                printf("{\"name\":\"%s\",\"type\":\"%s\",\"millidegrees\":",
                       entry->d_name, type);
                if (temperature[0]) printf("%s", temperature); else fputs("null", stdout);
                putchar('}');
            } else {
                printf("  %s: %s %s m°C\n", entry->d_name,
                       type[0] ? type : "unknown",
                       temperature[0] ? temperature : "n/a");
            }
            first = 0;
        }
        closedir(directory);
    }
    if (json) putchar(']');
    else if (first) puts("  unavailable");
}

static void print_cpu_governors(int json) {
    DIR *directory = opendir("/sys/devices/system/cpu");
    if (!json) puts("CPU frequency governors:");
    if (json) fputs(",\"cpu_governors\":[", stdout);
    int first = 1;
    if (directory) {
        struct dirent *entry;
        while ((entry = readdir(directory)) != NULL) {
            if (strncmp(entry->d_name, "cpu", 3) != 0 ||
                entry->d_name[3] < '0' || entry->d_name[3] > '9') continue;
            char path[512], governor[64] = "";
            snprintf(path, sizeof path,
                     "/sys/devices/system/cpu/%s/cpufreq/scaling_governor", entry->d_name);
            if (read_value(path, governor, sizeof governor) < 0) continue;
            if (json) {
                if (!first) putchar(',');
                printf("{\"cpu\":\"%s\",\"governor\":\"%s\"}",
                       entry->d_name, governor);
            } else printf("  %s: %s\n", entry->d_name, governor);
            first = 0;
        }
        closedir(directory);
    }
    if (json) putchar(']');
    else if (first) puts("  unavailable");
}

static void print_platform_profile(int json) {
    char profile[64] = "";
    if (read_value("/sys/firmware/acpi/platform_profile", profile, sizeof profile) < 0)
        snprintf(profile, sizeof profile, "unavailable");
    if (json) {
        fputs(",\"platform_profile\":", stdout);
        printf("\"%s\"", profile);
    } else printf("Platform profile: %s\n", profile);
}

static int show_status(int json) {
    if (json) putchar('{');
    else puts("Mintosh I/O Power Services");
    print_power_supplies(json);
    print_thermal_zones(json);
    print_cpu_governors(json);
    print_platform_profile(json);
    if (json) puts("}");
    return 0;
}

static int set_governor(const char *cpu_text, const char *governor) {
    unsigned int cpu;
    if (iops_parse_cpu_id(cpu_text, &cpu) < 0 || !iops_valid_token(governor)) {
        fputs("usage: iopsctl governor CPU GOVERNOR\n", stderr);
        return 64;
    }
    if (geteuid() != 0) {
        fputs("iopsctl: changing CPU governors requires root privileges\n", stderr);
        return 2;
    }
    char base[512], path[600], available[256];
    snprintf(base, sizeof base, "/sys/devices/system/cpu/cpu%u/cpufreq", cpu);
    snprintf(path, sizeof path, "%s/available_governors", base);
    if (read_value(path, available, sizeof available) < 0) {
        perror("iopsctl: available governors");
        return 1;
    }
    if (!iops_choice_available(governor, available)) {
        fprintf(stderr, "iopsctl: governor '%s' is not available for CPU %u\n",
                governor, cpu);
        return 64;
    }
    snprintf(path, sizeof path, "%s/scaling_governor", base);
    if (write_value(path, governor) < 0) {
        perror("iopsctl: set governor");
        return 1;
    }
    printf("CPU %u governor set to %s.\n", cpu, governor);
    return 0;
}

static int set_profile(const char *profile) {
    if (!iops_valid_token(profile)) {
        fputs("usage: iopsctl profile PROFILE\n", stderr);
        return 64;
    }
    if (geteuid() != 0) {
        fputs("iopsctl: changing platform profiles requires root privileges\n", stderr);
        return 2;
    }
    char available[256];
    if (read_value("/sys/firmware/acpi/platform_profile_choices",
                   available, sizeof available) < 0) {
        perror("iopsctl: platform profile choices");
        return 1;
    }
    if (!iops_choice_available(profile, available)) {
        fprintf(stderr, "iopsctl: profile '%s' is not available\n", profile);
        return 64;
    }
    if (write_value("/sys/firmware/acpi/platform_profile", profile) < 0) {
        perror("iopsctl: set platform profile");
        return 1;
    }
    printf("Platform profile set to %s.\n", profile);
    return 0;
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        puts("Mintosh I/O Power Services 0.1.0");
        return 0;
    }
    if (argc == 2 && strcmp(argv[1], "status") == 0) return show_status(0);
    if (argc == 3 && strcmp(argv[1], "status") == 0 &&
        strcmp(argv[2], "--json") == 0) return show_status(1);
    if (argc == 4 && strcmp(argv[1], "governor") == 0)
        return set_governor(argv[2], argv[3]);
    if (argc == 3 && strcmp(argv[1], "profile") == 0)
        return set_profile(argv[2]);
    fputs("usage: iopsctl status [--json] | governor CPU GOVERNOR | profile PROFILE\n", stderr);
    return 64;
}

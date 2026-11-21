#include <dirent.h>
#include <stdio.h>
static void print_directory(const char *title, const char *path) {
    DIR *directory = opendir(path);
    printf("%s:\n", title);
    if (!directory) { puts("  unavailable"); return; }
    struct dirent *entry;
    while ((entry = readdir(directory)))
        if (entry->d_name[0] != '.') printf("  %s\n", entry->d_name);
    closedir(directory);
}
void ioplus_print_sysfs_inventory(void) {
    print_directory("Block devices", "/sys/class/block");
    print_directory("Network interfaces", "/sys/class/net");
    print_directory("USB devices", "/sys/bus/usb/devices");
    print_directory("Graphics devices", "/sys/class/drm");
}

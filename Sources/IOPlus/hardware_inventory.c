#include <stdio.h>
#include <string.h>

void ioplus_print_kernel(void);
void ioplus_print_sysfs_inventory(void);
void ioplus_print_memory(void);
void ioplus_print_json(void);
int ioplus_watch_uevents(void);

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "watch") == 0)
        return ioplus_watch_uevents();
    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        puts("I/O+ 0.2.0");
        return 0;
    }
    if (argc > 1 && strcmp(argv[1], "--json") == 0) {
        ioplus_print_json();
        return 0;
    }
    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        puts("Usage: ioplus [--json|watch|--version|--help]");
        return 0;
    }
    if (argc > 1) {
        fputs("Usage: ioplus [--json] [--version] [--help]\n", stderr);
        return 64;
    }
    ioplus_print_kernel();
    ioplus_print_sysfs_inventory();
    ioplus_print_memory();
    return 0;
}

#include <stdio.h>
#include <string.h>
void ioplus_print_kernel(void);
void ioplus_print_sysfs_inventory(void);
void ioplus_print_memory(void);
int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "--version") == 0) { puts("I/O+ 0.1.0"); return 0; }
    ioplus_print_kernel();
    ioplus_print_sysfs_inventory();
    ioplus_print_memory();
    return 0;
}

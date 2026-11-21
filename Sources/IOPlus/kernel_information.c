#include <stdio.h>
#include <sys/utsname.h>
void ioplus_print_kernel(void) {
    struct utsname information;
    if (uname(&information) == 0)
        printf("Kernel: %s %s (%s)\n", information.sysname, information.release, information.machine);
}

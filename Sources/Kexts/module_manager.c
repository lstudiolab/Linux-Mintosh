#include "module_policy.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int kexts_execute(char *const arguments[]);

static int valid_module_name(const char *name) {
    if (!name || !name[0]) return 0;
    for (const unsigned char *cursor = (const unsigned char *)name; *cursor; ++cursor)
        if (!isalnum(*cursor) && *cursor != '_' && *cursor != '-') return 0;
    return name[0] != '-';
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        puts("Kexts 0.1.0");
        return 0;
    }
    if (argc == 2 && strcmp(argv[1], "status") == 0) {
        int enforced = kexts_module_signatures_enforced();
        if (enforced == 1) puts("Kernel module signature enforcement: enabled");
        else if (enforced == 0) puts("Kernel module signature enforcement: disabled");
        else puts("Kernel module signature enforcement: unavailable");
        return enforced == 1 ? 0 : 1;
    }
    if (argc == 2 && strcmp(argv[1], "list") == 0) {
        char *arguments[] = {"lsmod", NULL};
        return kexts_execute(arguments);
    }
    if (argc == 3 && (!strcmp(argv[1], "load") || !strcmp(argv[1], "unload"))) {
        if (!valid_module_name(argv[2])) {
            fputs("kextctl: invalid kernel module name\n", stderr);
            return 64;
        }
        if (geteuid() != 0) {
            fputs("kextctl: load and unload require root privileges\n", stderr);
            return 2;
        }
        if (!strcmp(argv[1], "load")) {
            if (kexts_module_signatures_enforced() != 1) {
                fputs("kextctl: refusing to load modules because kernel signature enforcement is not confirmed\n", stderr);
                return 3;
            }
            char *arguments[] = {"modprobe", "--", argv[2], NULL};
            return kexts_execute(arguments);
        }
        char *arguments[] = {"modprobe", "-r", "--", argv[2], NULL};
        return kexts_execute(arguments);
    }
    fputs("usage: kextctl status|list|load MODULE|unload MODULE\n", stderr);
    return 64;
}

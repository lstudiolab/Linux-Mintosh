#include <stdio.h>
#include <string.h>
int kexts_execute(char *const arguments[]);
int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--version") == 0) { puts("Kexts 0.1.0"); return 0; }
    if (argc == 2 && strcmp(argv[1], "list") == 0) { char *arguments[] = {"lsmod", NULL}; return kexts_execute(arguments); }
    if (argc == 3 && strcmp(argv[1], "load") == 0) { char *arguments[] = {"modprobe", argv[2], NULL}; return kexts_execute(arguments); }
    if (argc == 3 && strcmp(argv[1], "unload") == 0) { char *arguments[] = {"modprobe", "-r", argv[2], NULL}; return kexts_execute(arguments); }
    fputs("usage: kextctl list|load MODULE|unload MODULE\n", stderr);
    return 64;
}

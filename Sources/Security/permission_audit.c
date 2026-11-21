#include <stdio.h>
#include <string.h>
#include <unistd.h>
int security_check_file_permissions(const char *path);
int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "--version") == 0) { puts("Mintosh Security Audit 0.1.0"); return 0; }
    if (geteuid() != 0) { fputs("security-audit: run as root\n", stderr); return 2; }
    int warnings = security_check_file_permissions("/etc/shadow") +
                   security_check_file_permissions("/etc/passwd") +
                   security_check_file_permissions("/etc/sudoers");
    puts(warnings ? "Audit completed with warnings." : "Basic permission audit passed.");
    return warnings ? 1 : 0;
}

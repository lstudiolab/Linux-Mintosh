#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

int security_check_file_permissions(const char *path, mode_t expected_mode);

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        puts("Mintosh Security Audit 0.2.0");
        return 0;
    }
    if (geteuid() != 0) {
        fputs("security-audit: run as root\n", stderr);
        return 2;
    }

    int warnings = 0;
    warnings += security_check_file_permissions("/etc/passwd", 0644);
    warnings += security_check_file_permissions("/etc/shadow", 0640);
    warnings += security_check_file_permissions("/etc/group", 0644);
    warnings += security_check_file_permissions("/etc/gshadow", 0640);
    warnings += security_check_file_permissions("/etc/sudoers", 0440);
    puts(warnings ? "Audit completed with warnings." : "Critical account-file permissions passed.");
    return warnings ? 1 : 0;
}

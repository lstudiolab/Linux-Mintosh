#include "permission_check.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        puts("Mintosh Security Audit 0.3.0");
        return 0;
    }
    if (argc != 1) {
        fputs("Usage: security-audit [--version]\n", stderr);
        return 64;
    }
    if (geteuid() != 0) {
        fputs("security-audit: run as root\n", stderr);
        return 2;
    }

    int warnings = 0;
    warnings += security_check_file_permissions("/etc/passwd", 0644, 0);
    warnings += security_check_file_permissions("/etc/shadow", 0640, 0);
    warnings += security_check_file_permissions("/etc/group", 0644, 0);
    warnings += security_check_file_permissions("/etc/gshadow", 0640, 0);
    warnings += security_check_file_permissions("/etc/sudoers", 0440, 0);
    warnings += security_check_directory_permissions("/etc/mintosh", 0);
    warnings += security_check_directory_permissions("/etc/mintosh/services", 0);
    warnings += security_check_service_directory("/etc/mintosh/services", 0);

    static const char *const executables[] = {
        "/usr/bin/darl", "/usr/bin/launch7762", "/usr/bin/launch-client", "/usr/bin/launch511",
        "/usr/bin/mintoshctl", "/usr/bin/mstore", "/usr/bin/kextctl",
        "/usr/bin/ioplus", "/usr/bin/hp-compat", "/usr/bin/mintosh-log", "/usr/bin/security-audit"
    };
    for (size_t index = 0; index < sizeof executables / sizeof executables[0]; ++index)
        warnings += security_check_file_permissions(executables[index], 0755, 0);

    puts(warnings ? "Audit completed with warnings." : "Mintosh security baseline passed.");
    return warnings ? 1 : 0;
}

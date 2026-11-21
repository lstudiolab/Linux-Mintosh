#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int mstore_valid_package_name(const char *name);
int mstore_execute(char *const arguments[]);

static int require_root(void) {
    if (geteuid() == 0) return 0;
    fputs("mstore: this operation changes the system; run it with sudo.\n", stderr);
    return 2;
}

static int run_packages(int argc, char **argv, const char *operation) {
    if (argc < 3) {
        fprintf(stderr, "usage: mstore %s PACKAGE...\n", operation);
        return 64;
    }
    for (int index = 2; index < argc; ++index) {
        if (!mstore_valid_package_name(argv[index])) {
            fprintf(stderr, "mstore: invalid package name: %s\n", argv[index]);
            return 64;
        }
    }
    int privilege = require_root();
    if (privilege) return privilege;
    char **arguments = calloc((size_t)argc + 5, sizeof *arguments);
    if (!arguments) { perror("mstore: allocate arguments"); return 1; }
    arguments[0] = "apt-get";
    arguments[1] = (char *)operation;
    arguments[2] = "--yes";
    int offset = 3;
    if (strcmp(operation, "install") == 0) arguments[offset++] = "--no-install-recommends";
    arguments[offset++] = "--";
    for (int index = 2; index < argc; ++index) arguments[offset++] = argv[index];
    arguments[offset] = NULL;
    return mstore_execute(arguments);
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        puts("Mintosh Package Manager 0.1.0");
        return 0;
    }
    if (argc < 2) {
        fputs("usage: mstore search QUERY | info PACKAGE | list | install PACKAGE... | remove PACKAGE... | update | upgrade\n", stderr);
        return 64;
    }
    if (strcmp(argv[1], "search") == 0 && argc == 3) {
        char *arguments[] = {"apt-cache", "search", "--names-only", "--", argv[2], NULL};
        return mstore_execute(arguments);
    }
    if (strcmp(argv[1], "info") == 0 && argc == 3) {
        if (!mstore_valid_package_name(argv[2])) {
            fprintf(stderr, "mstore: invalid package name: %s\n", argv[2]);
            return 64;
        }
        char *arguments[] = {"apt-cache", "show", argv[2], NULL};
        return mstore_execute(arguments);
    }
    if (strcmp(argv[1], "list") == 0 && argc == 2) {
        char *arguments[] = {"dpkg-query", "-W", "-f=\${binary:Package}\\t\${Version}\\n", NULL};
        return mstore_execute(arguments);
    }
    if (strcmp(argv[1], "install") == 0 || strcmp(argv[1], "remove") == 0)
        return run_packages(argc, argv, argv[1]);
    if (strcmp(argv[1], "update") == 0 && argc == 2) {
        int privilege = require_root();
        if (privilege) return privilege;
        char *arguments[] = {"apt-get", "update", NULL};
        return mstore_execute(arguments);
    }
    if (strcmp(argv[1], "upgrade") == 0 && argc == 2) {
        int privilege = require_root();
        if (privilege) return privilege;
        char *arguments[] = {"apt-get", "upgrade", "--yes", NULL};
        return mstore_execute(arguments);
    }
    fputs("usage: mstore search QUERY | info PACKAGE | list | install PACKAGE... | remove PACKAGE... | update | upgrade\n", stderr);
    return 64;
}

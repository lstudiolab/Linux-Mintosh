#define _POSIX_C_SOURCE 200809L
#include "libsys.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int make_path(char *output, size_t capacity,
                     const char *base, const char *suffix) {
    int length = snprintf(output, capacity, "%s/%s", base, suffix);
    return length >= 0 && (size_t)length < capacity ? 0 : -1;
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        puts("launch-client 0.1.0");
        return 0;
    }
    if (argc != 1) {
        fputs("Usage: launch-client [--version]\n", stderr);
        return 64;
    }
    const char *home = getenv("HOME");
    const char *runtime = getenv("XDG_RUNTIME_DIR");
    const char *config = getenv("XDG_CONFIG_HOME");
    if (!home || home[0] != '/' || !runtime || runtime[0] != '/') {
        fputs("launch-client: HOME and XDG_RUNTIME_DIR must be absolute paths\n", stderr);
        return 2;
    }
    char default_config[4096], service_directory[4096], runtime_directory[4096];
    if (!config || !config[0]) {
        if (make_path(default_config, sizeof default_config, home, ".config") < 0)
            return 1;
        config = default_config;
    }
    if (config[0] != '/' ||
        make_path(service_directory, sizeof service_directory, config, "mintosh/services") < 0 ||
        make_path(runtime_directory, sizeof runtime_directory, runtime, "mintosh") < 0) {
        fputs("launch-client: invalid or oversized XDG directory path\n", stderr);
        return 2;
    }
    if (ms_mkdir_p(service_directory, 0700) < 0 ||
        ms_mkdir_p(runtime_directory, 0700) < 0) {
        perror("launch-client: create user service directories");
        return 1;
    }
    struct stat service_metadata, runtime_metadata;
    if (lstat(service_directory, &service_metadata) < 0 ||
        lstat(runtime_directory, &runtime_metadata) < 0 ||
        !S_ISDIR(service_metadata.st_mode) || !S_ISDIR(runtime_metadata.st_mode) ||
        service_metadata.st_uid != getuid() || runtime_metadata.st_uid != getuid() ||
        (service_metadata.st_mode & 0022) || (runtime_metadata.st_mode & 0022)) {
        fputs("launch-client: refusing untrusted user service directories\n", stderr);
        return 1;
    }
    if (setenv("MINTOSH_SERVICE_DIR", service_directory, 1) < 0 ||
        setenv("MINTOSH_RUN_DIR", runtime_directory, 1) < 0) {
        perror("launch-client: set service environment");
        return 1;
    }
    char *const arguments[] = {"/usr/bin/launch7762", NULL};
    execv(arguments[0], arguments);
    perror("launch-client: exec launch7762");
    return 127;
}

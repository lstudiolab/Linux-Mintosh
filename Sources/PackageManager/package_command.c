#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int mstore_valid_package_name(const char *name);
int mstore_execute(char *const arguments[]);

enum mstore_format {
    MSTORE_FORMAT_UNKNOWN = 0,
    MSTORE_FORMAT_DEB,
    MSTORE_FORMAT_FLATPAK_BUNDLE,
    MSTORE_FORMAT_APPIMAGE,
    MSTORE_FORMAT_NATIVE_PKG,
    MSTORE_FORMAT_RPM
};

static const char *format_name(enum mstore_format format) {
    switch (format) {
        case MSTORE_FORMAT_DEB: return "Debian package (.deb)";
        case MSTORE_FORMAT_FLATPAK_BUNDLE: return "Flatpak bundle (.flatpak)";
        case MSTORE_FORMAT_APPIMAGE: return "AppImage (.AppImage)";
        case MSTORE_FORMAT_NATIVE_PKG: return "Mintosh package (.pkg)";
        case MSTORE_FORMAT_RPM: return "RPM package (.rpm)";
        default: return "Unknown package format";
    }
}

static enum mstore_format detect_format(const char *path) {
    const char *extension = strrchr(path, '.');
    if (!extension) return MSTORE_FORMAT_UNKNOWN;
    if (strcmp(extension, ".deb") == 0) return MSTORE_FORMAT_DEB;
    if (strcmp(extension, ".flatpak") == 0) return MSTORE_FORMAT_FLATPAK_BUNDLE;
    if (strcmp(extension, ".AppImage") == 0 || strcmp(extension, ".appimage") == 0)
        return MSTORE_FORMAT_APPIMAGE;
    if (strcmp(extension, ".pkg") == 0) return MSTORE_FORMAT_NATIVE_PKG;
    if (strcmp(extension, ".rpm") == 0) return MSTORE_FORMAT_RPM;
    return MSTORE_FORMAT_UNKNOWN;
}

static int resolve_regular_file(const char *input, char resolved[PATH_MAX]) {
    if (!input || !input[0] || !realpath(input, resolved)) {
        fprintf(stderr, "mstore: cannot resolve package file '%s': %s\n",
                input ? input : "", strerror(errno));
        return 0;
    }
    struct stat status;
    if (stat(resolved, &status) != 0 || !S_ISREG(status.st_mode)) {
        fprintf(stderr, "mstore: package input must be a regular file: %s\n", input);
        return 0;
    }
    return 1;
}

static int require_root(void) {
    if (geteuid() == 0) return 0;
    fputs("mstore: this operation changes the system; run it with sudo.\n", stderr);
    return 2;
}

static int install_file(const char *input) {
    char path[PATH_MAX];
    if (!resolve_regular_file(input, path)) return 64;
    enum mstore_format format = detect_format(path);
    if (format == MSTORE_FORMAT_DEB) {
        int privilege = require_root();
        if (privilege) return privilege;
        char *arguments[] = {
            "apt-get", "--yes", "--no-install-recommends", "install", path, NULL
        };
        return mstore_execute(arguments);
    }
    if (format == MSTORE_FORMAT_FLATPAK_BUNDLE) {
        int privilege = require_root();
        if (privilege) return privilege;
        char *arguments[] = {
            "flatpak", "install", "--system", "--noninteractive", "--bundle", path, NULL
        };
        return mstore_execute(arguments);
    }
    if (format == MSTORE_FORMAT_APPIMAGE) {
        fputs("mstore: AppImage is a portable application, not a system package. "
              "Run it directly as your user; managed AppImage integration is planned.\n", stderr);
        return 69;
    }
    if (format == MSTORE_FORMAT_NATIVE_PKG) {
        fputs("mstore: .pkg is reserved for the native Mintosh format; its signed package "
              "specification and installer are not implemented yet.\n", stderr);
        return 69;
    }
    if (format == MSTORE_FORMAT_RPM) {
        fputs("mstore: RPM installation is not enabled; dependency and script compatibility "
              "cannot currently be guaranteed.\n", stderr);
        return 69;
    }
    fputs("mstore: unsupported package extension. Supported installers: .deb and .flatpak.\n", stderr);
    return 65;
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
    arguments[1] = "--yes";
    int offset = 2;
    if (strcmp(operation, "install") == 0) arguments[offset++] = "--no-install-recommends";
    arguments[offset++] = (char *)operation;
    for (int index = 2; index < argc; ++index) arguments[offset++] = argv[index];
    arguments[offset] = NULL;
    return mstore_execute(arguments);
}

static int run_appimage(const char *input) {
    char path[PATH_MAX];
    if (!resolve_regular_file(input, path)) return 64;
    if (detect_format(path) != MSTORE_FORMAT_APPIMAGE) {
        fputs("mstore: run currently accepts only .AppImage files.\\n", stderr);
        return 65;
    }
    if (geteuid() == 0) {
        fputs("mstore: refusing to launch an AppImage as root; run it as your user.\\n", stderr);
        return 2;
    }
    if (access(path, X_OK) != 0) {
        fprintf(stderr, "mstore: AppImage is not executable: %s\\n", path);
        return 126;
    }
    char *arguments[] = {path, NULL};
    execv(path, arguments);
    fprintf(stderr, "mstore: cannot launch AppImage: %s\\n", strerror(errno));
    return errno == ENOENT ? 127 : 126;
}

static int show_usage(void) {
    fputs("usage: mstore search QUERY | info PACKAGE | list | install PACKAGE... | "
          "install-file FILE | format FILE | run FILE.AppImage | remove PACKAGE... | update | upgrade\n", stderr);
    return 64;
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        puts("Mintosh Package Manager 0.2.0");
        return 0;
    }
    if (argc < 2) return show_usage();
    if (strcmp(argv[1], "run") == 0 && argc == 3) return run_appimage(argv[2]);\n    if (strcmp(argv[1], "format") == 0 && argc == 3) {
        char path[PATH_MAX];
        if (!resolve_regular_file(argv[2], path)) return 64;
        enum mstore_format format = detect_format(path);
        printf("%s\n", format_name(format));
        return format == MSTORE_FORMAT_UNKNOWN ? 65 : 0;
    }
    if ((strcmp(argv[1], "install-file") == 0 && argc == 3) ||
        (strcmp(argv[1], "install") == 0 && argc == 3 && detect_format(argv[2]) != MSTORE_FORMAT_UNKNOWN))
        return install_file(argv[2]);
    if (strcmp(argv[1], "search") == 0 && argc == 3) {
        char *arguments[] = {"apt-cache", "--names-only", "search", argv[2], NULL};
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
        char *arguments[] = {"dpkg-query", "-W", "-f=${binary:Package}\\t${Version}\\n", NULL};
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
    return show_usage();
}

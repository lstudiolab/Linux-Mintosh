#define _GNU_SOURCE
#include "darl_protocol.h"
#include "libsys.h"
#include <errno.h>
#include <grp.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

static volatile sig_atomic_t shutdown_requested;
static void request_shutdown(int signal_number) { (void)signal_number; shutdown_requested = 1; }

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        puts("Darl 0.2.0");
        return 0;
    }
    if (argc != 1) {
        fputs("Usage: darl [--version]\n", stderr);
        return 64;
    }
    if (getpid() == 1) {
        fputs("Darl is not the PID 1 init process.\n", stderr);
        return 2;
    }

    const char *runtime_directory = ms_runtime_directory();
    const char *socket_path = ms_darl_socket_path();
    if (!socket_path[0] || ms_mkdir_p(runtime_directory, 0755) < 0) {
        perror("darl: runtime directory");
        return 1;
    }
    struct stat directory_metadata;
    if (lstat(runtime_directory, &directory_metadata) < 0 ||
        !S_ISDIR(directory_metadata.st_mode) ||
        directory_metadata.st_uid != geteuid() ||
        (directory_metadata.st_mode & 0022) != 0) {
        fputs("darl: refusing an untrusted runtime directory\n", stderr);
        return 1;
    }
    struct stat existing;
    if (lstat(socket_path, &existing) == 0) {
        if (!S_ISSOCK(existing.st_mode) || existing.st_uid != geteuid()) {
            fputs("darl: refusing to replace an untrusted socket path\n", stderr);
            return 1;
        }
        if (unlink(socket_path) < 0) {
            perror("darl: remove stale socket");
            return 1;
        }
    } else if (errno != ENOENT) {
        perror("darl: inspect socket path");
        return 1;
    }
    int server = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (server < 0) { perror("darl: socket"); return 1; }
    struct sockaddr_un address = {.sun_family = AF_UNIX};
    if (strlen(socket_path) >= sizeof address.sun_path) {
        fputs("darl: socket path too long\n", stderr);
        close(server);
        return 1;
    }
    strcpy(address.sun_path, socket_path);
    if (bind(server, (struct sockaddr *)&address, sizeof address) < 0) {
        perror("darl: bind");
        close(server);
        return 1;
    }

    struct group *mintosh_group = getgrnam("mintosh-services");
    gid_t socket_group = mintosh_group ? mintosh_group->gr_gid : getegid();
    if (chown(socket_path, geteuid(), socket_group) < 0 ||
        chmod(socket_path, 0660) < 0 || listen(server, 16) < 0) {
        perror("darl: configure socket");
        close(server);
        unlink(socket_path);
        return 1;
    }

    signal(SIGTERM, request_shutdown);
    signal(SIGINT, request_shutdown);
    signal(SIGPIPE, SIG_IGN);
    ms_log("darl", "info", "Darl started");
    while (!shutdown_requested) {
        fd_set readable;
        FD_ZERO(&readable);
        FD_SET(server, &readable);
        struct timeval timeout = {.tv_sec = 1};
        int ready = select(server + 1, &readable, NULL, NULL, &timeout);
        if (ready < 0) { if (errno == EINTR) continue; break; }
        if (ready == 0) continue;
        int client = accept4(server, NULL, NULL, SOCK_CLOEXEC);
        if (client < 0) { if (errno == EINTR) continue; break; }
        char request[128] = {0}, response[256];
        if (ms_read_line(client, request, sizeof request, 2000) > 0) {
            darl_format_response(request, response, sizeof response);
            (void)ms_write_all(client, response, strlen(response));
        } else {
            (void)ms_write_all(client, "ERROR invalid request\n", 22);
        }
        close(client);
    }
    close(server);
    unlink(socket_path);
    ms_log("darl", "info", "Darl stopped");
    return 0;
}

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int mintosh_darl_request(const char *command, char *response, size_t capacity);
int mintosh_service_request(const char *command, char *response, size_t capacity);
static int select_user_runtime(void) {
    const char *runtime = getenv("XDG_RUNTIME_DIR");
    if (!runtime || runtime[0] != '/') {
        fputs("mintoshctl: --user requires an absolute XDG_RUNTIME_DIR\n", stderr);
        return -1;
    }
    char path[4096];
    int length = snprintf(path, sizeof path, "%s/mintosh", runtime);
    if (length < 0 || (size_t)length >= sizeof path ||
        setenv("MINTOSH_RUN_DIR", path, 1) < 0) {
        fputs("mintoshctl: invalid user runtime directory\n", stderr);
        return -1;
    }
    return 0;
}
int main(int argc, char **argv) {
    char response[4096], request[160];
    if (argc >= 3 && strcmp(argv[1], "--user") == 0) {
        if (select_user_runtime() < 0) return 2;
        if (argc == 3 && strcmp(argv[2], "services") == 0) {
            if (mintosh_service_request("LIST", response, sizeof response) < 0) {
                perror("mintoshctl: user launch-client"); return 1;
            }
            fputs(response, stdout); return 0;
        }
        if (argc == 4 && (!strcmp(argv[2], "start") || !strcmp(argv[2], "stop") ||
                          !strcmp(argv[2], "restart"))) {
            int length = snprintf(request, sizeof request, "%s %s",
                !strcmp(argv[2], "start") ? "START" :
                !strcmp(argv[2], "stop") ? "STOP" : "RESTART", argv[3]);
            if (length < 0 || (size_t)length >= sizeof request) return 64;
            if (mintosh_service_request(request, response, sizeof response) < 0) {
                perror("mintoshctl: user launch-client"); return 1;
            }
            fputs(response, stdout); return strncmp(response, "OK", 2) != 0;
        }
        fputs("usage: mintoshctl --user services|start NAME|stop NAME|restart NAME\n", stderr);
        return 64;
    }
    if (argc == 2 && strcmp(argv[1], "--version") == 0) { puts("mintoshctl 0.1.0"); return 0; }
    if (argc == 2 && (!strcmp(argv[1], "status") || !strcmp(argv[1], "version"))) {
        if (mintosh_darl_request(!strcmp(argv[1], "status") ? "STATUS" : "VERSION", response, sizeof response) < 0) { perror("mintoshctl: Darl"); return 1; }
        fputs(response, stdout); return 0;
    }
    if (argc == 2 && !strcmp(argv[1], "services")) {
        if (mintosh_service_request("LIST", response, sizeof response) < 0) { perror("mintoshctl: launch7762"); return 1; }
        fputs(response, stdout); return 0;
    }
    if (argc == 3 && (!strcmp(argv[1], "start") || !strcmp(argv[1], "stop") || !strcmp(argv[1], "restart"))) {
        int length = snprintf(request, sizeof request, "%s %s",
            !strcmp(argv[1], "start") ? "START" : !strcmp(argv[1], "stop") ? "STOP" : "RESTART", argv[2]);
        if (length < 0 || (size_t)length >= sizeof request) return 64;
        if (mintosh_service_request(request, response, sizeof response) < 0) { perror("mintoshctl: launch7762"); return 1; }
        fputs(response, stdout); return strncmp(response, "OK", 2) != 0;
    }
    fputs("usage: mintoshctl status|version|services|start NAME|stop NAME|restart NAME\n", stderr);
    return 64;
}

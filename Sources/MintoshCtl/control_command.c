#include <stdio.h>
#include <string.h>
int mintosh_mosf_request(const char *command, char *response, size_t capacity);
int mintosh_service_request(const char *command, char *response, size_t capacity);
int main(int argc, char **argv) {
    char response[4096], request[160];
    if (argc == 2 && strcmp(argv[1], "--version") == 0) { puts("mintoshctl 0.1.0"); return 0; }
    if (argc == 2 && (!strcmp(argv[1], "status") || !strcmp(argv[1], "version"))) {
        if (mintosh_mosf_request(!strcmp(argv[1], "status") ? "STATUS" : "VERSION", response, sizeof response) < 0) { perror("mintoshctl: MOSF"); return 1; }
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

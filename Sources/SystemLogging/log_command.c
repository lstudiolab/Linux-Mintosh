#include <stdio.h>
#include <string.h>
void mintosh_log_write(const char *level, const char *message);
int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--version") == 0) { puts("Mintosh System Logging 0.1.0"); return 0; }
    if (argc < 3) { fputs("usage: mosflog info|warning|error|debug MESSAGE\n", stderr); return 64; }
    const char *level = argv[1];
    if (strcmp(level, "info") && strcmp(level, "warning") && strcmp(level, "error") && strcmp(level, "debug")) {
        fputs("mosflog: invalid level\n", stderr); return 64;
    }
    mintosh_log_write(level, argv[2]);
    return 0;
}

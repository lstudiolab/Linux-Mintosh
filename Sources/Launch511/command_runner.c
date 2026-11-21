#include <stdio.h>
int launch511_command_index(int argc, char **argv);
int launch511_exec(char **arguments);
int main(int argc, char **argv) {
    int command_index = launch511_command_index(argc, argv);
    if (command_index >= argc) { fputs("usage: launch511 [--] command [arguments...]\n", stderr); return 64; }
    return launch511_exec(&argv[command_index]);
}

#include <string.h>
int launch511_command_index(int argc, char **argv) {
    int index = 1;
    if (index < argc && strcmp(argv[index], "--") == 0) ++index;
    return index;
}

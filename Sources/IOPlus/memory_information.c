#include <stdio.h>
void ioplus_print_memory(void) {
    FILE *memory = fopen("/proc/meminfo", "r");
    if (!memory) return;
    char line[256];
    if (fgets(line, sizeof line, memory)) printf("Memory: %s", line);
    fclose(memory);
}

#include "power_control.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>

int iops_valid_token(const char *value) {
    if (!value || !value[0]) return 0;
    for (const unsigned char *p = (const unsigned char *)value; *p; ++p)
        if (!((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') ||
              (*p >= '0' && *p <= '9') || *p == '_' || *p == '-')) return 0;
    return 1;
}

int iops_choice_available(const char *choice, const char *available) {
    if (!iops_valid_token(choice) || !available) return 0;
    size_t wanted = strlen(choice);
    const char *cursor = available;
    while (*cursor) {
        while (*cursor == ' ' || *cursor == '\t' || *cursor == '\n' || *cursor == '\r')
            ++cursor;
        const char *start = cursor;
        while (*cursor && *cursor != ' ' && *cursor != '\t' &&
               *cursor != '\n' && *cursor != '\r') ++cursor;
        if ((size_t)(cursor - start) == wanted &&
            memcmp(start, choice, wanted) == 0) return 1;
    }
    return 0;
}

int iops_parse_cpu_id(const char *text, unsigned int *cpu) {
    if (!text || !text[0] || !cpu) return -1;
    for (const unsigned char *p = (const unsigned char *)text; *p; ++p)
        if (*p < '0' || *p > '9') return -1;
    char *end = NULL;
    unsigned long value = strtoul(text, &end, 10);
    if (!end || *end || value > UINT_MAX) return -1;
    *cpu = (unsigned int)value;
    return 0;
}

#include "module_policy.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

int kexts_signature_value_enforced(const char *value) {
    if (!value) return -1;
    while (isspace((unsigned char)*value)) ++value;
    size_t length = strlen(value);
    while (length && isspace((unsigned char)value[length - 1])) --length;
    if (length == 1 && (value[0] == 'Y' || value[0] == 'y' || value[0] == '1'))
        return 1;
    if (length == 1 && (value[0] == 'N' || value[0] == 'n' || value[0] == '0'))
        return 0;
    return -1;
}

int kexts_module_signatures_enforced(void) {
    char value[32];
    FILE *file = fopen("/sys/module/module/parameters/sig_enforce", "r");
    if (!file) return -1;
    if (!fgets(value, sizeof value, file)) {
        fclose(file);
        return -1;
    }
    fclose(file);
    return kexts_signature_value_enforced(value);
}

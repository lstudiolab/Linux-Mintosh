#include "Foundation.h"
#include <stdlib.h>
#include <string.h>

char *mf_path_join(const char *base, const char *component) {
    if (!base || !component) return NULL;
    size_t base_length = strlen(base), component_length = strlen(component);
    int needs_separator = base_length > 0 && base[base_length - 1] != '/';
    if (component[0] == '/') { while (component[0] == '/') { ++component; --component_length; } }
    char *path = malloc(base_length + component_length + (size_t)needs_separator + 1);
    if (!path) return NULL;
    memcpy(path, base, base_length);
    if (needs_separator) path[base_length++] = '/';
    memcpy(path + base_length, component, component_length);
    path[base_length + component_length] = '\0';
    return path;
}

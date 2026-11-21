#include "Foundation.h"
#include <stdlib.h>
#include <string.h>

int mf_path_is_absolute(const char *path) {
    return path && path[0] == '/';
}

/* Lexical normalization only: this does not resolve symbolic links. */
char *mf_path_normalize(const char *path) {
    if (!path || !path[0]) return NULL;
    size_t length = strlen(path);
    if (length > (size_t)-1 / sizeof(size_t) - 1) return NULL;

    char *output = malloc(length + 2);
    size_t *starts = malloc((length + 1) * sizeof *starts);
    if (!output || !starts) {
        free(output);
        free(starts);
        return NULL;
    }

    int absolute = mf_path_is_absolute(path);
    size_t out = 0, depth = 0, cursor = 0;
    if (absolute) output[out++] = '/';

    while (cursor < length) {
        while (cursor < length && path[cursor] == '/') ++cursor;
        size_t start = cursor;
        while (cursor < length && path[cursor] != '/') ++cursor;
        size_t part_length = cursor - start;
        if (!part_length || (part_length == 1 && path[start] == '.')) continue;
        if (part_length == 2 && path[start] == '.' && path[start + 1] == '.') {
            if (depth && !(out - starts[depth - 1] == 2 &&
                           output[starts[depth - 1]] == '.' &&
                           output[starts[depth - 1] + 1] == '.')) {
                out = starts[--depth];
                if (out > (absolute ? 1u : 0u) && output[out - 1] == '/') --out;
            } else if (!absolute) {
                if (out && output[out - 1] != '/') output[out++] = '/';
                starts[depth++] = out;
                output[out++] = '.';
                output[out++] = '.';
            }
            continue;
        }
        if (out && output[out - 1] != '/') output[out++] = '/';
        starts[depth++] = out;
        memcpy(output + out, path + start, part_length);
        out += part_length;
    }

    if (out == 0) output[out++] = '.';
    output[out] = '\0';
    free(starts);
    return output;
}

char *mf_path_join(const char *base, const char *component) {
    if (!base || !component) return NULL;
    if (mf_path_is_absolute(component)) return mf_path_normalize(component);
    size_t a = strlen(base), b = strlen(component);
    if (a > (size_t)-1 - b - 2) return NULL;
    char *combined = malloc(a + b + 2);
    if (!combined) return NULL;
    memcpy(combined, base, a);
    size_t offset = a;
    if (offset && combined[offset - 1] != '/') combined[offset++] = '/';
    memcpy(combined + offset, component, b + 1);
    char *normalized = mf_path_normalize(combined);
    free(combined);
    return normalized;
}

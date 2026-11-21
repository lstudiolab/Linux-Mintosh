#include "Foundation.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

MFString mf_string_copy(const char *text) {
    MFString value = {0};
    if (!text) return value;
    size_t length = strlen(text);
    value.data = malloc(length + 1);
    if (!value.data) return value;
    memcpy(value.data, text, length + 1);
    value.length = length;
    return value;
}

int mf_string_append(MFString *value, const char *text) {
    if (!value || !text || (value->length && !value->data)) return -1;
    size_t extra = strlen(text);
    if (extra > (size_t)-1 - value->length - 1) return -1;
    size_t source_offset = 0;
    int source_aliases_value = 0;
    if (value->data) {
        uintptr_t base = (uintptr_t)value->data;
        uintptr_t source = (uintptr_t)text;
        if (source >= base && source - base <= value->length) {
            source_offset = (size_t)(source - base);
            source_aliases_value = 1;
        }
    }
    char *updated = realloc(value->data, value->length + extra + 1);
    if (!updated) return -1;
    const char *source = source_aliases_value ? updated + source_offset : text;
    memmove(updated + value->length, source, extra);
    updated[value->length + extra] = '\0';
    value->data = updated;
    value->length += extra;
    return 0;
}

void mf_string_destroy(MFString *value) {
    if (!value) return;
    free(value->data);
    value->data = NULL;
    value->length = 0;
}

int mf_string_equal(const MFString *left, const MFString *right) {
    return left && right && left->data && right->data &&
           left->length == right->length &&
           memcmp(left->data, right->data, left->length) == 0;
}

int mf_string_equal_cstr(const MFString *value, const char *text) {
    if (!value || !value->data || !text) return 0;
    size_t length = strlen(text);
    return value->length == length && memcmp(value->data, text, length) == 0;
}

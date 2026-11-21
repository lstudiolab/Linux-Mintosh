#include "Foundation.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

MFData mf_data_copy(const void *bytes, size_t length) {
    MFData value = {0};
    if (!length) return value;
    if (!bytes) return value;
    value.bytes = malloc(length);
    if (!value.bytes) return value;
    memcpy(value.bytes, bytes, length);
    value.length = length;
    return value;
}

int mf_data_append(MFData *value, const void *bytes, size_t length) {
    if (!value || (value->length && !value->bytes) || (length && !bytes))
        return -1;
    if (!length) return 0;
    if (length > (size_t)-1 - value->length) return -1;

    size_t source_offset = 0;
    int source_aliases_value = 0;
    if (value->bytes) {
        uintptr_t base = (uintptr_t)value->bytes;
        uintptr_t source = (uintptr_t)bytes;
        if (source >= base && source - base <= value->length) {
            source_offset = (size_t)(source - base);
            if (length > value->length - source_offset) return -1;
            source_aliases_value = 1;
        }
    }

    size_t old_length = value->length;
    unsigned char *updated = realloc(value->bytes, old_length + length);
    if (!updated) return -1;
    const unsigned char *source = source_aliases_value
        ? updated + source_offset : bytes;
    memmove(updated + old_length, source, length);
    value->bytes = updated;
    value->length = old_length + length;
    return 0;
}

void mf_data_destroy(MFData *value) {
    if (!value) return;
    free(value->bytes);
    value->bytes = NULL;
    value->length = 0;
}

int mf_data_equal(const MFData *left, const MFData *right) {
    return left && right && left->length == right->length &&
           (left->length == 0 ||
            (left->bytes && right->bytes &&
             memcmp(left->bytes, right->bytes, left->length) == 0));
}

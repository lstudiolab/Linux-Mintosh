#include "Foundation.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int mf_array_init(MFArray *array, size_t element_size) {
    if (!array || !element_size) return -1;
    *array = (MFArray){.element_size = element_size};
    return 0;
}

int mf_array_append(MFArray *array, const void *element) {
    if (!array || !element || !array->element_size) return -1;
    if (array->count == SIZE_MAX / array->element_size) return -1;
    if (array->count == array->capacity) {
        size_t capacity = array->capacity ? array->capacity : 8;
        if (capacity > SIZE_MAX / 2) capacity = array->count + 1;
        else capacity *= 2;
        if (capacity > SIZE_MAX / array->element_size) return -1;
        void *updated = realloc(array->elements, capacity * array->element_size);
        if (!updated) return -1;
        array->elements = updated;
        array->capacity = capacity;
    }
    memcpy((unsigned char *)array->elements + array->count * array->element_size,
           element, array->element_size);
    ++array->count;
    return 0;
}

void *mf_array_get(MFArray *array, size_t index) {
    if (!array || index >= array->count || !array->elements) return NULL;
    return (unsigned char *)array->elements + index * array->element_size;
}

const void *mf_array_get_const(const MFArray *array, size_t index) {
    if (!array || index >= array->count || !array->elements) return NULL;
    return (const unsigned char *)array->elements + index * array->element_size;
}

int mf_array_remove(MFArray *array, size_t index) {
    if (!array || index >= array->count || !array->elements) return -1;
    size_t trailing = array->count - index - 1;
    if (trailing)
        memmove((unsigned char *)array->elements + index * array->element_size,
                (unsigned char *)array->elements + (index + 1) * array->element_size,
                trailing * array->element_size);
    --array->count;
    return 0;
}

void mf_array_clear(MFArray *array) {
    if (array) array->count = 0;
}

void mf_array_destroy(MFArray *array) {
    if (!array) return;
    free(array->elements);
    *array = (MFArray){0};
}

#include "Foundation.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int mf_dictionary_init(MFDictionary *dictionary) {
    if (!dictionary) return -1;
    *dictionary = (MFDictionary){0};
    return 0;
}

static size_t find_key(const MFDictionary *dictionary, const char *key) {
    if (!dictionary || !key) return SIZE_MAX;
    for (size_t index = 0; index < dictionary->count; ++index)
        if (mf_string_equal_cstr(&dictionary->entries[index].key, key))
            return index;
    return SIZE_MAX;
}

int mf_dictionary_set(MFDictionary *dictionary, const char *key,
                      const char *value) {
    if (!dictionary || !key || !key[0] || !value ||
        dictionary->count > dictionary->capacity ||
        (dictionary->capacity && !dictionary->entries)) return -1;

    MFString copied_value = mf_string_copy(value);
    if (!copied_value.data) return -1;
    size_t existing = find_key(dictionary, key);
    if (existing != SIZE_MAX) {
        mf_string_destroy(&dictionary->entries[existing].value);
        dictionary->entries[existing].value = copied_value;
        return 0;
    }

    MFString copied_key = mf_string_copy(key);
    if (!copied_key.data) {
        mf_string_destroy(&copied_value);
        return -1;
    }
    if (dictionary->count == dictionary->capacity) {
        size_t capacity = dictionary->capacity ? dictionary->capacity * 2 : 8;
        if (capacity < dictionary->capacity ||
            capacity > SIZE_MAX / sizeof *dictionary->entries) {
            mf_string_destroy(&copied_key);
            mf_string_destroy(&copied_value);
            return -1;
        }
        void *updated = realloc(dictionary->entries,
                                capacity * sizeof *dictionary->entries);
        if (!updated) {
            mf_string_destroy(&copied_key);
            mf_string_destroy(&copied_value);
            return -1;
        }
        dictionary->entries = updated;
        dictionary->capacity = capacity;
    }
    dictionary->entries[dictionary->count++] =
        (MFDictionaryEntry){.key = copied_key, .value = copied_value};
    return 0;
}

const char *mf_dictionary_get(const MFDictionary *dictionary, const char *key) {
    size_t index = find_key(dictionary, key);
    if (index == SIZE_MAX) return NULL;
    return dictionary->entries[index].value.data;
}

int mf_dictionary_remove(MFDictionary *dictionary, const char *key) {
    size_t index = find_key(dictionary, key);
    if (index == SIZE_MAX) return -1;
    mf_string_destroy(&dictionary->entries[index].key);
    mf_string_destroy(&dictionary->entries[index].value);
    size_t trailing = dictionary->count - index - 1;
    if (trailing)
        memmove(&dictionary->entries[index], &dictionary->entries[index + 1],
                trailing * sizeof *dictionary->entries);
    --dictionary->count;
    return 0;
}

void mf_dictionary_clear(MFDictionary *dictionary) {
    if (!dictionary) return;
    for (size_t index = 0; index < dictionary->count; ++index) {
        mf_string_destroy(&dictionary->entries[index].key);
        mf_string_destroy(&dictionary->entries[index].value);
    }
    dictionary->count = 0;
}

void mf_dictionary_destroy(MFDictionary *dictionary) {
    if (!dictionary) return;
    mf_dictionary_clear(dictionary);
    free(dictionary->entries);
    *dictionary = (MFDictionary){0};
}

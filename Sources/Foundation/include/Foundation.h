#ifndef MINTOSH_FOUNDATION_H
#define MINTOSH_FOUNDATION_H

#include <stddef.h>
#include <sys/types.h>

typedef struct {
    char *data;
    size_t length;
} MFString;

typedef struct {
    unsigned char *bytes;
    size_t length;
} MFData;

typedef struct {
    void *elements;
    size_t count;
    size_t capacity;
    size_t element_size;
} MFArray;

typedef struct {
    MFString key;
    MFString value;
} MFDictionaryEntry;

typedef struct {
    MFDictionaryEntry *entries;
    size_t count;
    size_t capacity;
} MFDictionary;

typedef struct {
    char *scheme;
    char *authority;
    char *host;
    char *path;
    char *query;
    char *fragment;
    unsigned int port;
    int has_authority;
    int has_port;
} MFURL;

typedef enum {
    MF_ERROR_NONE = 0,
    MF_ERROR_INVALID_ARGUMENT,
    MF_ERROR_OUT_OF_MEMORY,
    MF_ERROR_IO,
    MF_ERROR_LIMIT,
    MF_ERROR_PERMISSION,
    MF_ERROR_NOT_FOUND,
    MF_ERROR_ALREADY_EXISTS,
    MF_ERROR_UNSUPPORTED
} MFErrorCode;

typedef struct {
    MFErrorCode code;
    int system_code;
    char message[160];
} MFError;

MFString mf_string_copy(const char *text);
int mf_string_append(MFString *value, const char *text);
void mf_string_destroy(MFString *value);
int mf_string_equal(const MFString *left, const MFString *right);
int mf_string_equal_cstr(const MFString *value, const char *text);

MFData mf_data_copy(const void *bytes, size_t length);
int mf_data_append(MFData *value, const void *bytes, size_t length);
void mf_data_destroy(MFData *value);
int mf_data_equal(const MFData *left, const MFData *right);

int mf_array_init(MFArray *array, size_t element_size);
int mf_array_append(MFArray *array, const void *element);
void *mf_array_get(MFArray *array, size_t index);
const void *mf_array_get_const(const MFArray *array, size_t index);
int mf_array_remove(MFArray *array, size_t index);
void mf_array_clear(MFArray *array);
void mf_array_destroy(MFArray *array);

int mf_dictionary_init(MFDictionary *dictionary);
int mf_dictionary_set(MFDictionary *dictionary, const char *key, const char *value);
const char *mf_dictionary_get(const MFDictionary *dictionary, const char *key);
int mf_dictionary_remove(MFDictionary *dictionary, const char *key);
void mf_dictionary_clear(MFDictionary *dictionary);
void mf_dictionary_destroy(MFDictionary *dictionary);

int mf_url_parse(const char *text, MFURL *url, MFError *error);
int mf_url_is_secure(const MFURL *url);
void mf_url_destroy(MFURL *url);

void mf_error_clear(MFError *error);
void mf_error_set(MFError *error, MFErrorCode code, int system_code,
                  const char *message);

char *mf_path_join(const char *base, const char *component);
char *mf_path_normalize(const char *path);
int mf_path_is_absolute(const char *path);

MFData mf_file_read(const char *path, size_t maximum_size, MFError *error);
int mf_file_write_atomic(const char *path, const void *bytes, size_t length,
                         mode_t mode, MFError *error);

#endif

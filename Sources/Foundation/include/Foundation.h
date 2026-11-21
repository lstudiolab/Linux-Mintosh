#ifndef MINTOSH_FOUNDATION_H
#define MINTOSH_FOUNDATION_H

#include <stddef.h>

typedef struct {
    char *data;
    size_t length;
} MFString;

MFString mf_string_copy(const char *text);
int mf_string_append(MFString *value, const char *text);
void mf_string_destroy(MFString *value);
int mf_string_equal(const MFString *left, const MFString *right);
int mf_string_equal_cstr(const MFString *value, const char *text);

char *mf_path_join(const char *base, const char *component);
char *mf_path_normalize(const char *path);
int mf_path_is_absolute(const char *path);

#endif

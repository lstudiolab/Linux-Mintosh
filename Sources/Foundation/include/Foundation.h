#ifndef MINTOSH_FOUNDATION_H
#define MINTOSH_FOUNDATION_H
#include <stddef.h>
typedef struct{char *data;size_t length;} MFString;
MFString mf_string_copy(const char *text);
void mf_string_destroy(MFString *value);
int mf_string_equal(const MFString *a,const MFString *b);
char *mf_path_join(const char *base,const char *component);
#endif

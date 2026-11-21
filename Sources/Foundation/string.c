#include "Foundation.h"
#include <stdlib.h>
#include <string.h>

MFString mf_string_copy(const char *text) {
    MFString value = {0};
    if (!text) return value;
    value.length = strlen(text);
    value.data = malloc(value.length + 1);
    if (!value.data) { value.length = 0; return value; }
    memcpy(value.data, text, value.length + 1);
    return value;
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

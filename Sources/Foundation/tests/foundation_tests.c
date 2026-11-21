#include "Foundation.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    MFString value = mf_string_copy("Mint");
    assert(value.data && value.length == 4);
    assert(mf_string_append(&value, "osh") == 0);
    assert(mf_string_equal_cstr(&value, "Mintosh"));
    assert(mf_string_append(&value, value.data + 3) == 0);
    assert(mf_string_equal_cstr(&value, "Mintoshtosh"));
    MFString same = mf_string_copy("Mintosh");
    assert(mf_string_equal(&value, &same));
    mf_string_destroy(&same);
    mf_string_destroy(&value);
    assert(value.data == NULL && value.length == 0);

    MFData bytes = mf_data_copy("abc", 3);
    assert(bytes.bytes && bytes.length == 3);
    assert(mf_data_append(&bytes, bytes.bytes + 1, 2) == 0);
    MFData expected = mf_data_copy("abcbc", 5);
    assert(mf_data_equal(&bytes, &expected));
    mf_data_destroy(&expected);
    mf_data_destroy(&bytes);
    assert(bytes.bytes == NULL && bytes.length == 0);

    char *path = mf_path_join("/usr//local", "bin/../sbin");
    assert(path && strcmp(path, "/usr/local/sbin") == 0);
    free(path);
    path = mf_path_normalize("../../var/./log");
    assert(path && strcmp(path, "../../var/log") == 0);
    free(path);
    path = mf_path_join("/usr/local", "/etc/../var");
    assert(path && strcmp(path, "/var") == 0);
    free(path);
    assert(mf_path_is_absolute("/etc"));
    assert(!mf_path_is_absolute("etc"));
    return 0;
}

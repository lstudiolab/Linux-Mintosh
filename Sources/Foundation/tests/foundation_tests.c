#define _POSIX_C_SOURCE 200809L
#include "Foundation.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main(void) {
    MFString value = mf_string_copy("Mint");
    assert(value.data && value.length == 4);
    assert(mf_string_append(&value, "osh") == 0);
    assert(mf_string_equal_cstr(&value, "Mintosh"));
    assert(mf_string_append(&value, value.data + 3) == 0);
    assert(mf_string_equal_cstr(&value, "Mintoshtosh"));
    MFString same = mf_string_copy("Mintoshtosh");
    assert(mf_string_equal(&value, &same));
    mf_string_destroy(&same); mf_string_destroy(&value);
    assert(value.data == NULL && value.length == 0);

    MFData bytes = mf_data_copy("abc", 3);
    assert(bytes.bytes && bytes.length == 3);
    assert(mf_data_append(&bytes, bytes.bytes + 1, 2) == 0);
    MFData expected = mf_data_copy("abcbc", 5);
    assert(mf_data_equal(&bytes, &expected));
    mf_data_destroy(&expected); mf_data_destroy(&bytes);

    int numbers[] = {4, 8, 15};
    MFArray array;
    assert(mf_array_init(&array, sizeof(int)) == 0);
    for (size_t i = 0; i < 8; ++i)
        assert(mf_array_append(&array, &numbers[i % 3]) == 0);
    int first = *(int *)mf_array_get(&array, 0);
    assert(mf_array_append(&array, mf_array_get(&array, 0)) == 0);
    assert(array.count == 9 && *(int *)mf_array_get(&array, 8) == first);
    assert(mf_array_append(&array, mf_array_get(&array, 2)) == 0);
    assert(array.count == 10 && *(int *)mf_array_get(&array, 9) == numbers[2]);
    assert(mf_array_remove(&array, 1) == 0);
    assert(array.count == 9 && *(const int *)mf_array_get_const(&array, 1) == numbers[2]);
    assert(mf_array_get(&array, 9) == NULL);
    mf_array_clear(&array); assert(array.count == 0);
    mf_array_destroy(&array); assert(array.elements == NULL);

    MFError error;
    MFDictionary dictionary;
    assert(mf_dictionary_init(&dictionary) == 0);
    assert(mf_dictionary_set(&dictionary, "distribution", "Mintosh") == 0);
    assert(mf_dictionary_set(&dictionary, "kernel", "Linux") == 0);
    assert(strcmp(mf_dictionary_get(&dictionary, "distribution"), "Mintosh") == 0);
    assert(mf_dictionary_set(&dictionary, "kernel", "LMK") == 0);
    assert(strcmp(mf_dictionary_get(&dictionary, "kernel"), "LMK") == 0);
    assert(mf_dictionary_remove(&dictionary, "distribution") == 0);
    assert(mf_dictionary_get(&dictionary, "distribution") == NULL);
    mf_dictionary_destroy(&dictionary);
    assert(dictionary.entries == NULL && dictionary.count == 0);

    MFURL url = {0};
    assert(mf_url_parse("HTTPS://Example.COM:443/system/api?mode=full#status",
                        &url, &error) == 0);
    assert(strcmp(url.scheme, "https") == 0);
    assert(strcmp(url.host, "example.com") == 0);
    assert(url.has_port && url.port == 443);
    assert(strcmp(url.path, "/system/api") == 0);
    assert(strcmp(url.query, "mode=full") == 0);
    assert(strcmp(url.fragment, "status") == 0);
    assert(mf_url_is_secure(&url));
    mf_url_destroy(&url);
    assert(mf_url_parse("file:///etc/mintosh-release", &url, &error) == 0);
    assert(strcmp(url.scheme, "file") == 0);
    assert(strcmp(url.path, "/etc/mintosh-release") == 0);
    assert(!mf_url_is_secure(&url));
    mf_url_destroy(&url);
    assert(mf_url_parse("https://example.com:99999/", &url, &error) < 0);
    assert(error.code == MF_ERROR_INVALID_ARGUMENT);
    assert(mf_url_parse("https://example.com/%Q0", &url, &error) < 0);
    assert(mf_url_parse("https://example.com/%", &url, &error) < 0);
    assert(mf_url_parse("https://example.com/%A", &url, &error) < 0);
    assert(mf_url_parse("relative/path", &url, &error) < 0);

    mf_error_set(&error, MF_ERROR_IO, 5, "test");
    assert(error.code == MF_ERROR_IO && error.system_code == 5);
    mf_error_clear(&error); assert(error.code == MF_ERROR_NONE);

    char directory[] = "/tmp/mintosh-foundation-XXXXXX";
    assert(mkdtemp(directory));
    char path[256], link_path[256];
    assert(snprintf(path, sizeof path, "%s/state.bin", directory) > 0);
    assert(snprintf(link_path, sizeof link_path, "%s/link.bin", directory) > 0);
    assert(mf_file_write_atomic(path, "state", 5, 0600, &error) == 0);
    struct stat metadata;
    assert(stat(path, &metadata) == 0 && (metadata.st_mode & 0777) == 0600);
    MFData loaded = mf_file_read(directory, 16, &error);
    assert(!loaded.bytes && error.code == MF_ERROR_IO && error.system_code == EINVAL);
    loaded = mf_file_read(path, 16, &error);
    assert(loaded.length == 5 && memcmp(loaded.bytes, "state", 5) == 0);
    mf_data_destroy(&loaded);
    loaded = mf_file_read(path, 4, &error);
    assert(!loaded.bytes && error.code == MF_ERROR_LIMIT);
    assert(symlink(path, link_path) == 0);
    loaded = mf_file_read(link_path, 16, &error);
    assert(!loaded.bytes && error.code == MF_ERROR_IO);
    assert(unlink(link_path) == 0 && unlink(path) == 0 && rmdir(directory) == 0);

    char *normalized = mf_path_join("/usr//local", "bin/../sbin");
    assert(normalized && strcmp(normalized, "/usr/local/sbin") == 0); free(normalized);
    normalized = mf_path_normalize("../../var/./log");
    assert(normalized && strcmp(normalized, "../../var/log") == 0); free(normalized);
    normalized = mf_path_join("/usr/local", "/etc/../var");
    assert(normalized && strcmp(normalized, "/var") == 0); free(normalized);
    assert(mf_path_is_absolute("/etc") && !mf_path_is_absolute("etc"));
    return 0;
}

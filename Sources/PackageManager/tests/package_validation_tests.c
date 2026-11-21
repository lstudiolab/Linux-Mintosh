#include <assert.h>
#include <string.h>

int mstore_valid_package_name(const char *name);

int main(void) {
    assert(mstore_valid_package_name("firefox"));
    assert(mstore_valid_package_name("libc6"));
    assert(mstore_valid_package_name("linux-image-amd64:amd64"));
    assert(mstore_valid_package_name("a+b.c-1"));
    assert(!mstore_valid_package_name(""));
    assert(!mstore_valid_package_name("-option"));
    assert(!mstore_valid_package_name("Firefox"));
    assert(!mstore_valid_package_name("pkg;rm"));
    assert(!mstore_valid_package_name("pkg:"));
    assert(!mstore_valid_package_name("pkg:AMD64"));
    assert(!mstore_valid_package_name("pkg:amd64:extra"));
    assert(!mstore_valid_package_name("pkg name"));
    return 0;
}

#include <stddef.h>
#include <string.h>

static int package_character(char value) {
    return (value >= 'a' && value <= 'z') ||
           (value >= '0' && value <= '9') ||
           value == '+' || value == '.' || value == '-';
}

static int architecture_character(char value) {
    return (value >= 'a' && value <= 'z') ||
           (value >= '0' && value <= '9') || value == '-';
}

int mstore_valid_package_name(const char *name) {
    if (!name || !name[0]) return 0;
    const char *separator = strchr(name, ':');
    size_t package_length = separator ? (size_t)(separator - name) : strlen(name);
    if (package_length == 0 || package_length > 64 ||
        !((name[0] >= 'a' && name[0] <= 'z') ||
          (name[0] >= '0' && name[0] <= '9'))) return 0;

    for (size_t index = 0; index < package_length; ++index)
        if (!package_character(name[index])) return 0;

    if (!separator) return 1;
    const char *architecture = separator + 1;
    size_t architecture_length = strlen(architecture);
    if (architecture_length == 0 || architecture_length > 32 ||
        !((architecture[0] >= 'a' && architecture[0] <= 'z') ||
          (architecture[0] >= '0' && architecture[0] <= '9'))) return 0;
    for (size_t index = 0; index < architecture_length; ++index)
        if (!architecture_character(architecture[index])) return 0;
    return 1;
}

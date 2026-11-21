int mstore_valid_package_name(const char *name) {
    if (!name || !name[0]) return 0;
    int index = 0;
    if (!((name[index] >= 'a' && name[index] <= 'z') ||
          (name[index] >= '0' && name[index] <= '9'))) return 0;
    for (; name[index] && name[index] != ':'; ++index) {
        char value = name[index];
        if (!((value >= 'a' && value <= 'z') ||
              (value >= '0' && value <= '9') ||
              value == '+' || value == '.' || value == '-')) return 0;
    }
    if (name[index] == ':') {
        ++index;
        if (!((name[index] >= 'a' && name[index] <= 'z') ||
              (name[index] >= '0' && name[index] <= '9'))) return 0;
        for (; name[index]; ++index) {
            char value = name[index];
            if (!((value >= 'a' && value <= 'z') ||
                  (value >= '0' && value <= '9') || value == '-')) return 0;
        }
    }
    return name[index] == '\0';
}

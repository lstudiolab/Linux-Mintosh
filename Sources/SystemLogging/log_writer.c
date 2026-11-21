#include "libsys.h"
void mintosh_log_write(const char *level, const char *message) {
    ms_log("mintosh", level, message);
}

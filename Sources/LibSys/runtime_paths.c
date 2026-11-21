#include "libsys.h"
#include <stdio.h>
#include <stdlib.h>

const char *ms_runtime_directory(void) {
    const char *override = getenv("MINTOSH_RUN_DIR");
    return override && override[0] ? override : "/run/mintosh";
}

static const char *socket_path(const char *filename) {
    static char path[4096];
    int length = snprintf(path, sizeof path, "%s/%s", ms_runtime_directory(), filename);
    return length < 0 || (size_t)length >= sizeof path ? "" : path;
}

const char *ms_mosf_socket_path(void) { return socket_path("mosf.sock"); }
const char *ms_launch_socket_path(void) { return socket_path("launch.sock"); }

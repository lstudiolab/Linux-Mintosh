#define _POSIX_C_SOURCE 200809L
#include "libsys.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main(void) {
    char root[] = "/tmp/mintosh-libsys-XXXXXX";
    assert(mkdtemp(root));
    char nested[512], link_path[512], file_path[512];
    assert(snprintf(nested, sizeof nested, "%s/a/b/c", root) > 0);
    assert(ms_mkdir_p(nested, 0700) == 0);
    struct stat metadata;
    assert(stat(nested, &metadata) == 0 && S_ISDIR(metadata.st_mode));
    assert(snprintf(link_path, sizeof link_path, "%s/link", root) > 0);
    assert(symlink("/tmp", link_path) == 0);
    char through_link[512];
    assert(snprintf(through_link, sizeof through_link, "%s/link/should-not-exist", root) > 0);
    errno = 0;
    assert(ms_mkdir_p(through_link, 0700) < 0 && (errno == ELOOP || errno == ENOTDIR));
    assert(snprintf(file_path, sizeof file_path, "%s/a/b/c/file", root) > 0);
    FILE *file = fopen(file_path, "w");
    assert(file); fclose(file);
    errno = 0;
    assert(ms_mkdir_p(file_path, 0700) < 0 && errno == ENOTDIR);
    errno = 0;
    assert(ms_mkdir_p("../unsafe", 0700) < 0 && errno == EINVAL);
    assert(unlink(file_path) == 0);
    assert(unlink(link_path) == 0);
    char path[512];
    assert(snprintf(path, sizeof path, "%s/a/b/c", root) > 0);
    assert(rmdir(path) == 0);
    assert(snprintf(path, sizeof path, "%s/a/b", root) > 0);
    assert(rmdir(path) == 0);
    assert(snprintf(path, sizeof path, "%s/a", root) > 0);
    assert(rmdir(path) == 0);
    assert(rmdir(root) == 0);
    return 0;
}

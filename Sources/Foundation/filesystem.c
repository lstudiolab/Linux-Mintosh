#define _GNU_SOURCE
#include "Foundation.h"
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static void set_io_error(MFError *error, int code, int system_code, const char *message) {
    mf_error_set(error, (MFErrorCode)code, system_code, message);
}

MFData mf_file_read(const char *path, size_t maximum_size, MFError *error) {
    mf_error_clear(error);
    MFData result = {0};
    if (!path || !path[0] || !maximum_size) {
        set_io_error(error, MF_ERROR_INVALID_ARGUMENT, EINVAL, "invalid file read arguments");
        return result;
    }
    int descriptor = open(path, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (descriptor < 0) {
        int saved = errno;
        set_io_error(error, saved == EACCES ? MF_ERROR_PERMISSION :
                     saved == ENOENT ? MF_ERROR_NOT_FOUND : MF_ERROR_IO,
                     saved, "unable to open file");
        return result;
    }
    struct stat metadata;
    if (fstat(descriptor, &metadata) < 0 || !S_ISREG(metadata.st_mode)) {
        int saved = errno ? errno : EINVAL;
        close(descriptor);
        set_io_error(error, MF_ERROR_IO, saved, "input is not a regular file");
        return result;
    }
    if (metadata.st_size < 0 || (uintmax_t)metadata.st_size > maximum_size) {
        close(descriptor);
        set_io_error(error, MF_ERROR_LIMIT, EFBIG, "file exceeds configured size limit");
        return result;
    }
    size_t capacity = metadata.st_size > 0 ? (size_t)metadata.st_size : 4096;
    if (capacity > maximum_size) capacity = maximum_size;
    unsigned char *buffer = malloc(capacity ? capacity : 1);
    if (!buffer) {
        close(descriptor);
        set_io_error(error, MF_ERROR_OUT_OF_MEMORY, ENOMEM, "unable to allocate file buffer");
        return result;
    }
    size_t length = 0;
    for (;;) {
        if (length == capacity) {
            if (capacity == maximum_size) {
                unsigned char extra;
                ssize_t count = read(descriptor, &extra, 1);
                if (count > 0) {
                    free(buffer); close(descriptor);
                    set_io_error(error, MF_ERROR_LIMIT, EFBIG, "file exceeds configured size limit");
                    return result;
                }
                if (count < 0 && errno == EINTR) continue;
                if (count < 0) {
                    int saved = errno; free(buffer); close(descriptor);
                    set_io_error(error, MF_ERROR_IO, saved, "file read failed");
                    return result;
                }
                break;
            }
            size_t next = capacity > maximum_size / 2 ? maximum_size : capacity * 2;
            unsigned char *updated = realloc(buffer, next);
            if (!updated) {
                free(buffer); close(descriptor);
                set_io_error(error, MF_ERROR_OUT_OF_MEMORY, ENOMEM, "unable to grow file buffer");
                return result;
            }
            buffer = updated;
            capacity = next;
        }
        ssize_t count = read(descriptor, buffer + length, capacity - length);
        if (count < 0 && errno == EINTR) continue;
        if (count < 0) {
            int saved = errno; free(buffer); close(descriptor);
            set_io_error(error, MF_ERROR_IO, saved, "file read failed");
            return result;
        }
        if (count == 0) break;
        length += (size_t)count;
    }
    close(descriptor);
    if (!length) { free(buffer); return result; }
    unsigned char *shrunk = realloc(buffer, length);
    result.bytes = shrunk ? shrunk : buffer;
    result.length = length;
    return result;
}

static int write_all(int descriptor, const void *bytes, size_t length) {
    const unsigned char *cursor = bytes;
    while (length) {
        ssize_t count = write(descriptor, cursor, length);
        if (count < 0 && errno == EINTR) continue;
        if (count <= 0) { if (!count) errno = EIO; return -1; }
        cursor += (size_t)count;
        length -= (size_t)count;
    }
    return 0;
}

int mf_file_write_atomic(const char *path, const void *bytes, size_t length,
                         mode_t mode, MFError *error) {
    mf_error_clear(error);
    if (!path || !path[0] || (length && !bytes) || (mode & ~0777) != 0) {
        set_io_error(error, MF_ERROR_INVALID_ARGUMENT, EINVAL, "invalid atomic write arguments");
        return -1;
    }
    char *copy = strdup(path);
    if (!copy) {
        set_io_error(error, MF_ERROR_OUT_OF_MEMORY, ENOMEM, "unable to allocate path");
        return -1;
    }
    char *slash = strrchr(copy, '/');
    char *directory = ".";
    char *filename = copy;
    if (slash) {
        *slash = '\0';
        directory = copy[0] ? copy : "/";
        filename = slash + 1;
    }
    if (!filename[0] || strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0) {
        free(copy);
        set_io_error(error, MF_ERROR_INVALID_ARGUMENT, EINVAL, "invalid destination filename");
        return -1;
    }
    size_t template_size = strlen(directory) + strlen(filename) + 16;
    char *temporary = malloc(template_size);
    if (!temporary) {
        free(copy);
        set_io_error(error, MF_ERROR_OUT_OF_MEMORY, ENOMEM, "unable to allocate temporary path");
        return -1;
    }
    snprintf(temporary, template_size, "%s/.%s.tmp.XXXXXX", directory, filename);
    int descriptor = mkstemp(temporary);
    if (descriptor < 0) {
        int saved = errno; free(temporary); free(copy);
        set_io_error(error, saved == EACCES ? MF_ERROR_PERMISSION : MF_ERROR_IO,
                     saved, "unable to create temporary file");
        return -1;
    }
    int result = 0;
    if (fchmod(descriptor, mode) < 0 || write_all(descriptor, bytes, length) < 0 ||
        fsync(descriptor) < 0) result = -1;
    int saved = errno;
    if (close(descriptor) < 0 && result == 0) { result = -1; saved = errno; }
    if (result == 0 && rename(temporary, path) < 0) { result = -1; saved = errno; }
    if (result == 0) {
        int directory_fd = open(directory, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
        if (directory_fd < 0) {
            result = -1;
            saved = errno;
        } else {
            if (fsync(directory_fd) < 0) {
                result = -1;
                saved = errno;
            }
            if (close(directory_fd) < 0 && result == 0) {
                result = -1;
                saved = errno;
            }
        }
    }
    if (result < 0) {
        unlink(temporary);
        set_io_error(error, saved == EACCES ? MF_ERROR_PERMISSION : MF_ERROR_IO,
                     saved, "atomic file write failed");
    }
    free(temporary);
    free(copy);
    return result;
}

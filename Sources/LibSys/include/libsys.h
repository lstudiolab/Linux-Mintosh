#ifndef MINTOSH_LIBSYS_H
#define MINTOSH_LIBSYS_H
#include <stddef.h>
#include <sys/types.h>

const char *ms_runtime_directory(void);
const char *ms_darl_socket_path(void);
const char *ms_launch_socket_path(void);
int ms_mkdir_p(const char *path, mode_t mode);
int ms_write_all(int descriptor, const void *data, size_t length);
int ms_read_line(int descriptor, char *buffer, size_t capacity, int timeout_ms);
int ms_send_command(const char *socket_path, const char *command, char *reply, size_t capacity);
void ms_log(const char *component, const char *level, const char *message);
#endif

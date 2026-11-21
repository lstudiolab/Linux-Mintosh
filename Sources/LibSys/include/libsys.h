#ifndef MINTOSH_LIBSYS_H
#define MINTOSH_LIBSYS_H
#include <stddef.h>
#include <sys/types.h>
#define MINTOSH_RUNTIME_DIR "/run/mintosh"
#define MINTOSH_MOSF_SOCKET MINTOSH_RUNTIME_DIR "/mosf.sock"
#define MINTOSH_LAUNCH_SOCKET MINTOSH_RUNTIME_DIR "/launch.sock"
int ms_mkdir_p(const char *, mode_t);
int ms_write_all(int, const void *, size_t);
int ms_send_command(const char *, const char *, char *, size_t);
void ms_log(const char *, const char *, const char *);
#endif

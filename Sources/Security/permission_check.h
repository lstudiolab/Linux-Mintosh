#ifndef MINTOSH_SECURITY_PERMISSION_CHECK_H
#define MINTOSH_SECURITY_PERMISSION_CHECK_H
#include <sys/stat.h>
#include <sys/types.h>

int security_check_file_group_permissions(const char *path, mode_t expected_mode,\n                                            uid_t expected_owner, gid_t expected_group);\nint security_check_socket_permissions(const char *path, uid_t expected_owner,\n                                      gid_t expected_group, mode_t expected_mode);\nint security_check_file_permissions(const char *path, mode_t expected_mode,
                                    uid_t expected_owner);
int security_check_directory_permissions(const char *path, uid_t expected_owner);
int security_check_service_directory(const char *path, uid_t expected_owner);
#endif

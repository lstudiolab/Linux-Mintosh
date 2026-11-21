#ifndef MINTOSH_IOPLUS_UEVENT_H
#define MINTOSH_IOPLUS_UEVENT_H
#include <stddef.h>

typedef struct {
    char action[16];
    char devpath[512];
    char subsystem[64];
    char devtype[64];
    char driver[64];
    char product[64];
    char interface_name[128];
    char sequence[32];
} IOPLUSUEvent;

int ioplus_parse_uevent(const char *buffer, size_t length, IOPLUSUEvent *event);
int ioplus_watch_uevents(void);
#endif

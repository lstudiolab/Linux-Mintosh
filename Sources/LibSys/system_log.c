#include "libsys.h"
#include <string.h>
#include <syslog.h>

void ms_log(const char *component, const char *level, const char *message) {
    int priority = LOG_INFO;
    if (strcmp(level, "error") == 0) priority = LOG_ERR;
    else if (strcmp(level, "warning") == 0) priority = LOG_WARNING;
    else if (strcmp(level, "debug") == 0) priority = LOG_DEBUG;
    openlog(component, LOG_PID | LOG_NDELAY, LOG_DAEMON);
    syslog(priority, "%s", message);
    closelog();
}

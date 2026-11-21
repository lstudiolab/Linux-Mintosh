#include "uevent.h"
#include <string.h>

static int copy_value(char *destination, size_t capacity,
                      const char *source, size_t length) {
    if (length >= capacity) return -1;
    memcpy(destination, source, length);
    destination[length] = '\0';
    return 0;
}
static int valid_action(const char *action) {
    static const char *const actions[] = {
        "add", "remove", "change", "move", "online", "offline",
        "bind", "unbind", NULL
    };
    for (size_t i = 0; actions[i]; ++i)
        if (strcmp(action, actions[i]) == 0) return 1;
    return 0;
}
int ioplus_parse_uevent(const char *buffer, size_t length, IOPLUSUEvent *event) {
    if (!buffer || !length || !event) return -1;
    *event = (IOPLUSUEvent){0};
    size_t offset = 0;
    int first = 1;
    while (offset < length) {
        const char *token = buffer + offset;
        const char *end = memchr(token, '\0', length - offset);
        size_t token_length = end ? (size_t)(end - token) : length - offset;
        if (!token_length) break;
        if (first) {
            const char *separator = memchr(token, '@', token_length);
            if (separator) {
                size_t action_length = (size_t)(separator - token);
                size_t path_length = token_length - action_length - 1;
                if (copy_value(event->action, sizeof event->action, token, action_length) < 0 ||
                    copy_value(event->devpath, sizeof event->devpath, separator + 1, path_length) < 0)
                    return -1;
            }
            first = 0;
        } else {
            const char *separator = memchr(token, '=', token_length);
            if (separator) {
                size_t key_length = (size_t)(separator - token);
                const char *value = separator + 1;
                size_t value_length = token_length - key_length - 1;
                char *destination = NULL;
                size_t capacity = 0;
                if (key_length == 6 && memcmp(token, "ACTION", 6) == 0) { destination = event->action; capacity = sizeof event->action; }
                else if (key_length == 7 && memcmp(token, "DEVPATH", 7) == 0) { destination = event->devpath; capacity = sizeof event->devpath; }
                else if (key_length == 9 && memcmp(token, "SUBSYSTEM", 9) == 0) { destination = event->subsystem; capacity = sizeof event->subsystem; }
                else if (key_length == 7 && memcmp(token, "DEVTYPE", 7) == 0) { destination = event->devtype; capacity = sizeof event->devtype; }
                else if (key_length == 6 && memcmp(token, "DRIVER", 6) == 0) { destination = event->driver; capacity = sizeof event->driver; }
                else if (key_length == 7 && memcmp(token, "PRODUCT", 7) == 0) { destination = event->product; capacity = sizeof event->product; }
                else if (key_length == 9 && memcmp(token, "INTERFACE", 9) == 0) { destination = event->interface_name; capacity = sizeof event->interface_name; }
                else if (key_length == 6 && memcmp(token, "SEQNUM", 6) == 0) { destination = event->sequence; capacity = sizeof event->sequence; }
                if (destination && copy_value(destination, capacity, value, value_length) < 0)
                    return -1;
            }
        }
        offset += token_length + (end ? 1u : 0u);
    }
    return event->action[0] && event->devpath[0] && valid_action(event->action) ? 0 : -1;
}

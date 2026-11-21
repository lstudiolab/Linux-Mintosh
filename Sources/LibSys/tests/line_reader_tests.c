#include "libsys.h"
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    int channel[2];
    char line[16];
    assert(pipe(channel) == 0);
    assert(write(channel[1], "PING\\n", 5) == 5);
    close(channel[1]);
    assert(ms_read_line(channel[0], line, sizeof line, 100) == 4);
    assert(strcmp(line, "PING") == 0);
    close(channel[0]);

    assert(pipe(channel) == 0);
    assert(write(channel[1], "12345\\n", 6) == 6);
    assert(ms_read_line(channel[0], line, 5, 100) < 0 && errno == EMSGSIZE);
    close(channel[0]);
    close(channel[1]);

    assert(pipe(channel) == 0);
    assert(ms_read_line(channel[0], line, sizeof line, 20) < 0 && errno == ETIMEDOUT);
    close(channel[0]);
    close(channel[1]);
    return 0;
}

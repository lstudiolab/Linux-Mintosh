#define _GNU_SOURCE
#include <xcu/xcu.h>

#include <assert.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
    int ports[2];
    assert(xcu_port_pair(ports) == 0);
    const char message[] = "XCU";
    assert(xcu_port_send(ports[0], message, sizeof(message), -1) ==
           (ssize_t)sizeof(message));
    char received[sizeof(message)] = {0};
    assert(xcu_port_receive(ports[1], received, sizeof(received), NULL) ==
           (ssize_t)sizeof(received));
    assert(memcmp(message, received, sizeof(message)) == 0);
    close(ports[0]);
    close(ports[1]);
    return 0;
}

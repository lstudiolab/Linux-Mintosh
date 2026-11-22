#define _GNU_SOURCE
#include <xcu/xcu.h>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

int main(void)
{
    xcu_task task;
    errno = 0;
    assert(xcu_task_open(-1, &task) == -1 && errno == EINVAL);

    int ports[2];
    assert(xcu_port_pair(ports) == 0);
    const char message[] = "xcu-message";
    assert(xcu_port_send(ports[0], message, sizeof(message), -1) == (ssize_t)sizeof(message));
    char received[64] = {0};
    assert(xcu_port_receive(ports[1], received, sizeof(received), NULL) == (ssize_t)sizeof(message));
    assert(memcmp(message, received, sizeof(message)) == 0);
    close(ports[0]);
    close(ports[1]);

    int memory = xcu_memory_create("xcu-test", 4096);
    assert(memory >= 0);
    close(memory);

    int queue = xcu_event_queue_create();
    assert(queue >= 0);
    int pipefd[2];
    assert(pipe(pipefd) == 0);
    assert(xcu_event_queue_add(queue, pipefd[0], EPOLLIN, 0x584355U) == 0);
    assert(write(pipefd[1], "x", 1) == 1);
    xcu_event event;
    assert(xcu_event_queue_wait(queue, &event, 1, 1000) == 1);
    assert(event.token == 0x584355U);
    assert((event.events & EPOLLIN) != 0);
    assert(xcu_event_queue_remove(queue, pipefd[0]) == 0);
    close(pipefd[0]);
    close(pipefd[1]);
    close(queue);

    puts("XCU primitive tests passed.");
    return 0;
}

#define _GNU_SOURCE
#include <xcu/xcu.h>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/ipc.h>
#include <sys/msg.h>
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

    struct xcu_test_message {
        long type;
        char text[32];
    } sent = { .type = 1, .text = "bsd-sysv-ipc" }, received_message = {0};
    int message_queue = msgget(IPC_PRIVATE, IPC_CREAT | 0600);
    assert(message_queue >= 0);
    assert(msgsnd(message_queue, &sent, sizeof(sent.text), 0) == 0);
    assert(msgrcv(message_queue, &received_message, sizeof(received_message.text), 1, 0)
           == (ssize_t)sizeof(sent.text));
    assert(strcmp(received_message.text, sent.text) == 0);
    assert(msgctl(message_queue, IPC_RMID, NULL) == 0);

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

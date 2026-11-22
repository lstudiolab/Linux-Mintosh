#define _GNU_SOURCE
#include "../include/xnv/xnv.h"

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <unistd.h>

int main(void)
{
    xnv_task task = { .pid = -1, .pidfd = -1 };
    assert(xnv_task_open(getpid(), &task) == 0);
    assert(task.pid == getpid() && task.pidfd >= 0);
    assert(xnv_task_signal(&task, 0) == 0);
    xnv_task_close(&task);
    assert(task.pidfd == -1);

    int ports[2];
    assert(xnv_port_pair(ports) == 0);
    const char message[] = "XNV port";
    assert(xnv_port_send(ports[0], message, sizeof(message), -1) ==
           (ssize_t)sizeof(message));
    char received[32] = {0};
    assert(xnv_port_receive(ports[1], received, sizeof(received), NULL) ==
           (ssize_t)sizeof(message));
    assert(memcmp(received, message, sizeof(message)) == 0);

    int pipe_descriptors[2];
    assert(pipe2(pipe_descriptors, O_CLOEXEC) == 0);
    assert(xnv_port_send(ports[0], "fd", 2, pipe_descriptors[0]) == 2);
    int transferred = -1;
    assert(xnv_port_receive(ports[1], received, sizeof(received), &transferred) == 2);
    assert(transferred >= 0);
    assert(write(pipe_descriptors[1], "p", 1) == 1);
    char byte = 0;
    assert(read(transferred, &byte, 1) == 1 && byte == 'p');
    close(transferred);
    close(pipe_descriptors[0]);
    close(pipe_descriptors[1]);
    close(ports[0]);
    close(ports[1]);

    int memory = xnv_memory_create("xnv-test", 4096);
    assert(memory >= 0);
    const char data[] = "memory object";
    assert(pwrite(memory, data, sizeof(data), 0) == (ssize_t)sizeof(data));
    char copy[sizeof(data)] = {0};
    assert(pread(memory, copy, sizeof(copy), 0) == (ssize_t)sizeof(copy));
    assert(memcmp(copy, data, sizeof(data)) == 0);
    int seals = fcntl(memory, F_GET_SEALS);
    assert(seals >= 0);
    assert((seals & (F_SEAL_GROW | F_SEAL_SHRINK | F_SEAL_SEAL)) ==
           (F_SEAL_GROW | F_SEAL_SHRINK | F_SEAL_SEAL));
    close(memory);

    int event_pipe[2];
    assert(pipe2(event_pipe, O_CLOEXEC | O_NONBLOCK) == 0);
    int queue = xnv_event_queue_create();
    assert(queue >= 0);
    assert(xnv_event_queue_add(queue, event_pipe[0], EPOLLIN, 0x584e56U) == 0);
    assert(write(event_pipe[1], "e", 1) == 1);
    xnv_event event;
    assert(xnv_event_queue_wait(queue, &event, 1, 1000) == 1);
    assert(event.token == 0x584e56U && (event.events & EPOLLIN) != 0);
    assert(xnv_event_queue_remove(queue, event_pipe[0]) == 0);
    close(queue);
    close(event_pipe[0]);
    close(event_pipe[1]);

    puts("XNV Linux primitive tests passed.");
    return 0;
}

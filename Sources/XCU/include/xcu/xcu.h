#ifndef MINTOSH_XCU_H
#define MINTOSH_XCU_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#define XCU_PORT_MAX_MESSAGE 65536U

typedef struct {
    pid_t pid;
    int pidfd;
} xcu_task;

typedef struct {
    uint32_t events;
    uint64_t token;
} xcu_event;

int xcu_task_open(pid_t pid, xcu_task *task);
int xcu_task_signal(const xcu_task *task, int signal_number);
void xcu_task_close(xcu_task *task);

int xcu_port_pair(int ports[2]);
ssize_t xcu_port_send(int port, const void *message, size_t size, int pass_fd);
ssize_t xcu_port_receive(int port, void *buffer, size_t capacity, int *receive_fd);

int xcu_memory_create(const char *name, size_t size);

int xcu_event_queue_create(void);
int xcu_event_queue_add(int queue, int descriptor, uint32_t events, uint64_t token);
int xcu_event_queue_remove(int queue, int descriptor);
int xcu_event_queue_wait(int queue, xcu_event *events, size_t capacity, int timeout_ms);

#endif

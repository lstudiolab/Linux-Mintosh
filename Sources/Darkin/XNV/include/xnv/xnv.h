#ifndef MINTOSH_XNV_H
#define MINTOSH_XNV_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#define XNV_PORT_MAX_MESSAGE 65536U

typedef struct {
    pid_t pid;
    int pidfd;
} xnv_task;

typedef struct {
    uint32_t events;
    uint64_t token;
} xnv_event;

/* Mach-inspired task handle backed by Linux pidfd. */
int xnv_task_open(pid_t pid, xnv_task *task);
int xnv_task_signal(const xnv_task *task, int signal_number);
void xnv_task_close(xnv_task *task);

/* Mach-inspired port pair backed by authenticated local sequenced packets.
 * pass_fd may be -1. receive_fd may be NULL when no descriptor is expected.
 */
int xnv_port_pair(int ports[2]);
ssize_t xnv_port_send(int port, const void *message, size_t size, int pass_fd);
ssize_t xnv_port_receive(int port, void *buffer, size_t capacity, int *receive_fd);

/* Mach-inspired memory object backed by a sealable Linux memfd. */
int xnv_memory_create(const char *name, size_t size);

/* Event delivery backed by Linux epoll. */
int xnv_event_queue_create(void);
int xnv_event_queue_add(int queue, int descriptor, uint32_t events, uint64_t token);
int xnv_event_queue_remove(int queue, int descriptor);
int xnv_event_queue_wait(int queue, xnv_event *events, size_t capacity, int timeout_ms);

#endif

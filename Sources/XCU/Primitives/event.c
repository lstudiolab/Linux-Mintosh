#define _GNU_SOURCE
#include <xcu/xcu.h>

#include <errno.h>
#include <sys/epoll.h>
#include <unistd.h>

int xcu_event_queue_create(void)
{
    return epoll_create1(EPOLL_CLOEXEC);
}

int xcu_event_queue_add(int queue, int descriptor, uint32_t events, uint64_t token)
{
    if (queue < 0 || descriptor < 0 || events == 0) {
        errno = EINVAL;
        return -1;
    }
    struct epoll_event event = { .events = events, .data.u64 = token };
    return epoll_ctl(queue, EPOLL_CTL_ADD, descriptor, &event);
}

int xcu_event_queue_remove(int queue, int descriptor)
{
    if (queue < 0 || descriptor < 0) { errno = EINVAL; return -1; }
    return epoll_ctl(queue, EPOLL_CTL_DEL, descriptor, NULL);
}

int xcu_event_queue_wait(int queue, xcu_event *events, size_t capacity, int timeout_ms)
{
    if (queue < 0 || events == NULL || capacity == 0 ||
        capacity > 1024U || timeout_ms < -1) {
        errno = EINVAL;
        return -1;
    }
    struct epoll_event native_events[1024];
    int count = epoll_wait(queue, native_events, (int)capacity, timeout_ms);
    if (count < 0) return -1;
    for (int i = 0; i < count; ++i) {
        events[i].events = native_events[i].events;
        events[i].token = native_events[i].data.u64;
    }
    return count;
}

#define _POSIX_C_SOURCE 200809L
#include "GVD.h"
#include <assert.h>
#include <errno.h>
#include <stdatomic.h>
#include <stddef.h>

static void count_task(void *context) {
    atomic_size_t *completed = context;
    atomic_fetch_add_explicit(completed, 1, memory_order_relaxed);
}
int main(void) {
    errno = 0;
    assert(gvd_queue_create(0, 8) == NULL && errno == EINVAL);
    assert(gvd_queue_create(2, 0) == NULL && errno == EINVAL);
    GVDQueue *queue = gvd_queue_create(4, 1024);
    assert(queue);
    atomic_size_t completed = 0;
    for (size_t index = 0; index < 1000; ++index)
        assert(gvd_queue_submit(queue, count_task, &completed) == 0);
    assert(gvd_queue_wait_idle(queue) == 0);
    assert(atomic_load_explicit(&completed, memory_order_relaxed) == 1000);
    assert(gvd_queue_pending(queue) == 0);
    assert(gvd_queue_shutdown(queue) == 0);
    errno = 0;
    assert(gvd_queue_submit(queue, count_task, &completed) < 0 && errno == ESHUTDOWN);
    gvd_queue_destroy(queue);
    return 0;
}

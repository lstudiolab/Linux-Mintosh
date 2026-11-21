#define _POSIX_C_SOURCE 200809L
#include "GVD.h"
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>

typedef struct GVDTask {
    GVDTaskFunction function;
    void *context;
    struct GVDTask *next;
} GVDTask;

struct GVDQueue {
    pthread_mutex_t mutex;
    pthread_cond_t available;
    pthread_cond_t idle;
    pthread_t *workers;
    size_t worker_count, maximum_queued, queued, active;
    GVDTask *head, *tail;
    int accepting;
};

static void *worker_main(void *context) {
    GVDQueue *queue = context;
    for (;;) {
        pthread_mutex_lock(&queue->mutex);
        while (!queue->head && queue->accepting)
            pthread_cond_wait(&queue->available, &queue->mutex);
        if (!queue->head && !queue->accepting) {
            pthread_mutex_unlock(&queue->mutex);
            return NULL;
        }
        GVDTask *task = queue->head;
        queue->head = task->next;
        if (!queue->head) queue->tail = NULL;
        --queue->queued;
        ++queue->active;
        pthread_mutex_unlock(&queue->mutex);

        task->function(task->context);
        free(task);

        pthread_mutex_lock(&queue->mutex);
        --queue->active;
        if (!queue->queued && !queue->active)
            pthread_cond_broadcast(&queue->idle);
        pthread_mutex_unlock(&queue->mutex);
    }
}

static void release_queue(GVDQueue *queue, size_t initialized_workers) {
    for (size_t index = 0; index < initialized_workers; ++index)
        pthread_join(queue->workers[index], NULL);
    pthread_cond_destroy(&queue->idle);
    pthread_cond_destroy(&queue->available);
    pthread_mutex_destroy(&queue->mutex);
    free(queue->workers);
    free(queue);
}

GVDQueue *gvd_queue_create(size_t worker_count, size_t maximum_queued) {
    if (!worker_count || worker_count > 128 ||
        !maximum_queued || maximum_queued > 65536) {
        errno = EINVAL;
        return NULL;
    }
    GVDQueue *queue = calloc(1, sizeof *queue);
    if (!queue) return NULL;
    queue->workers = calloc(worker_count, sizeof *queue->workers);
    if (!queue->workers) { free(queue); return NULL; }
    queue->worker_count = worker_count;
    queue->maximum_queued = maximum_queued;
    queue->accepting = 1;

    int error = pthread_mutex_init(&queue->mutex, NULL);
    if (error) goto fail_memory;
    error = pthread_cond_init(&queue->available, NULL);
    if (error) goto fail_mutex;
    error = pthread_cond_init(&queue->idle, NULL);
    if (error) goto fail_available;

    size_t created = 0;
    for (; created < worker_count; ++created) {
        error = pthread_create(&queue->workers[created], NULL, worker_main, queue);
        if (error) break;
    }
    if (created != worker_count) {
        pthread_mutex_lock(&queue->mutex);
        queue->accepting = 0;
        pthread_cond_broadcast(&queue->available);
        pthread_mutex_unlock(&queue->mutex);
        release_queue(queue, created);
        errno = error;
        return NULL;
    }
    return queue;

fail_available:
    pthread_cond_destroy(&queue->available);
fail_mutex:
    pthread_mutex_destroy(&queue->mutex);
fail_memory:
    free(queue->workers);
    free(queue);
    errno = error;
    return NULL;
}

int gvd_queue_submit(GVDQueue *queue, GVDTaskFunction function, void *context) {
    if (!queue || !function) { errno = EINVAL; return -1; }
    GVDTask *task = malloc(sizeof *task);
    if (!task) return -1;
    *task = (GVDTask){.function = function, .context = context};
    pthread_mutex_lock(&queue->mutex);
    if (!queue->accepting) {
        pthread_mutex_unlock(&queue->mutex);
        free(task);
        errno = ESHUTDOWN;
        return -1;
    }
    if (queue->queued >= queue->maximum_queued) {
        pthread_mutex_unlock(&queue->mutex);
        free(task);
        errno = EAGAIN;
        return -1;
    }
    if (queue->tail) queue->tail->next = task;
    else queue->head = task;
    queue->tail = task;
    ++queue->queued;
    pthread_cond_signal(&queue->available);
    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

int gvd_queue_wait_idle(GVDQueue *queue) {
    if (!queue) { errno = EINVAL; return -1; }
    pthread_mutex_lock(&queue->mutex);
    while (queue->queued || queue->active) {
        int error = pthread_cond_wait(&queue->idle, &queue->mutex);
        if (error) {
            pthread_mutex_unlock(&queue->mutex);
            errno = error;
            return -1;
        }
    }
    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

int gvd_queue_shutdown(GVDQueue *queue) {
    if (!queue) { errno = EINVAL; return -1; }
    pthread_mutex_lock(&queue->mutex);
    queue->accepting = 0;
    pthread_cond_broadcast(&queue->available);
    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

void gvd_queue_destroy(GVDQueue *queue) {
    if (!queue) return;
    (void)gvd_queue_shutdown(queue);
    release_queue(queue, queue->worker_count);
}

size_t gvd_queue_pending(GVDQueue *queue) {
    if (!queue) return 0;
    pthread_mutex_lock(&queue->mutex);
    size_t pending = queue->queued + queue->active;
    pthread_mutex_unlock(&queue->mutex);
    return pending;
}

#ifndef MINTOSH_GVD_H
#define MINTOSH_GVD_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct GVDQueue GVDQueue;
typedef void (*GVDTaskFunction)(void *context);
GVDQueue *gvd_queue_create(size_t worker_count, size_t maximum_queued);
int gvd_queue_submit(GVDQueue *queue, GVDTaskFunction function, void *context);
int gvd_queue_wait_idle(GVDQueue *queue);
int gvd_queue_shutdown(GVDQueue *queue);
void gvd_queue_destroy(GVDQueue *queue);
size_t gvd_queue_pending(GVDQueue *queue);
#ifdef __cplusplus
}
#endif
#endif

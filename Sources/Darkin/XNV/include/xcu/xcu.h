#ifndef MINTOSH_XCU_H
#define MINTOSH_XCU_H

#include <xnv/xnv.h>

/* Canonical XCU names. The legacy XNV ABI remains available during migration. */
typedef xnv_task xcu_task;
typedef xnv_event xcu_event;

#define XCU_PORT_MAX_MESSAGE XNV_PORT_MAX_MESSAGE

#define xcu_task_open xnv_task_open
#define xcu_task_signal xnv_task_signal
#define xcu_task_close xnv_task_close

#define xcu_port_pair xnv_port_pair
#define xcu_port_send xnv_port_send
#define xcu_port_receive xnv_port_receive

#define xcu_memory_create xnv_memory_create

#define xcu_event_queue_create xnv_event_queue_create
#define xcu_event_queue_add xnv_event_queue_add
#define xcu_event_queue_remove xnv_event_queue_remove
#define xcu_event_queue_wait xnv_event_queue_wait

#endif

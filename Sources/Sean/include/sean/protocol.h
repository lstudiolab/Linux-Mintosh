#ifndef LENIX_SEAN_PROTOCOL_H
#define LENIX_SEAN_PROTOCOL_H

#include <stdint.h>

#define SEAN_PROTOCOL_VERSION 1U
#define SEAN_MAX_PACKET_SIZE 4096U
#define SEAN_SYSTEM_SOCKET "/run/sean/system.sock"

enum sean_operation {
    SEAN_OP_NONE = 0,
    SEAN_OP_READ_SYSTEM_INFO = 1
};

enum sean_status {
    SEAN_STATUS_OK = 0,
    SEAN_STATUS_DENIED = 1,
    SEAN_STATUS_INVALID = 2,
    SEAN_STATUS_UNAVAILABLE = 3,
    SEAN_STATUS_INTERNAL_ERROR = 4
};

struct sean_request {
    uint32_t size;
    uint16_t version;
    uint16_t operation;
    uint64_t request_id;
};

struct sean_response {
    uint32_t size;
    uint16_t version;
    uint16_t status;
    uint64_t request_id;
    uint32_t payload_size;
    uint32_t reserved;
};

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(struct sean_request) == 16, "Sean request ABI changed");
_Static_assert(sizeof(struct sean_response) == 24, "Sean response ABI changed");
#endif

#endif

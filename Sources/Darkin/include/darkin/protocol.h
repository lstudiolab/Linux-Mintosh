#ifndef MINTOSH_DARKIN_PROTOCOL_H
#define MINTOSH_DARKIN_PROTOCOL_H

#include <stdint.h>

#define DARKIN_PROTOCOL_VERSION 1U
#define DARKIN_MAX_PACKET_SIZE 4096U
#define DARKIN_SYSTEM_SOCKET "/run/mintosh-darkin/system.sock"

enum darkin_operation {
    DARKIN_OP_NONE = 0,
    DARKIN_OP_READ_SYSTEM_INFO = 1
};

enum darkin_status {
    DARKIN_STATUS_OK = 0,
    DARKIN_STATUS_DENIED = 1,
    DARKIN_STATUS_INVALID = 2,
    DARKIN_STATUS_UNAVAILABLE = 3,
    DARKIN_STATUS_INTERNAL_ERROR = 4
};

struct darkin_request {
    uint32_t size;
    uint16_t version;
    uint16_t operation;
    uint64_t request_id;
};

struct darkin_response {
    uint32_t size;
    uint16_t version;
    uint16_t status;
    uint64_t request_id;
    uint32_t payload_size;
    uint32_t reserved;
};

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(struct darkin_request) == 16, "Darkin request ABI changed");
_Static_assert(sizeof(struct darkin_response) == 24, "Darkin response ABI changed");
#endif

#endif

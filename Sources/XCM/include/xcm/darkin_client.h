#ifndef MINTOSH_XCM_DARKIN_CLIENT_H
#define MINTOSH_XCM_DARKIN_CLIENT_H

#include <stddef.h>
#include <stdint.h>

int xcm_darkin_system_info(uint64_t request_id, char *buffer,
                           size_t capacity, uint32_t *payload_size);

#endif

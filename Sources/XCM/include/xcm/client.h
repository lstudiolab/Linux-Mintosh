#ifndef XCM_CLIENT_H
#define XCM_CLIENT_H

#include "protocol.h"
#include <stddef.h>

#define XCM_DEFAULT_SOCKET "/run/mintosh/xcm.sock"

/* Returns 0 for a valid XCM response, -1 for transport/protocol errors.
 * A non-OK response is returned normally in response->status.
 */
int xcm_call(const char *socket_path,
             const struct xcm_request *request,
             const void *payload,
             struct xcm_response *response,
             void *response_payload,
             size_t response_capacity);

#endif

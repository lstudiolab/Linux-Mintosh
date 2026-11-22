#define _GNU_SOURCE
#include "include/xcm/client.h"

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int xcm_call(const char *socket_path,
             const struct xcm_request *request,
             const void *payload,
             struct xcm_response *response,
             void *response_payload,
             size_t response_capacity)
{
    if (socket_path == NULL || request == NULL || response == NULL ||
        (request->payload_size != 0 && payload == NULL) ||
        (response_capacity != 0 && response_payload == NULL)) {
        errno = EINVAL;
        return -1;
    }
    size_t path_length = strlen(socket_path);
    if (path_length == 0 || path_length >= sizeof(((struct sockaddr_un *)0)->sun_path)) {
        errno = ENAMETOOLONG;
        return -1;
    }
    if (request->size != sizeof(*request) + request->payload_size ||
        request->size > XCM_MAX_MESSAGE_SIZE ||
        request->version != XCM_PROTOCOL_VERSION || request->request_id == 0) {
        errno = EINVAL;
        return -1;
    }

    int socket_fd = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0);
    if (socket_fd < 0) return -1;
    struct sockaddr_un address;
    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    memcpy(address.sun_path, socket_path, path_length + 1U);
    if (connect(socket_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        int saved = errno;
        close(socket_fd);
        errno = saved;
        return -1;
    }

    unsigned char packet[XCM_MAX_MESSAGE_SIZE];
    memcpy(packet, request, sizeof(*request));
    if (request->payload_size != 0)
        memcpy(packet + sizeof(*request), payload, request->payload_size);
    if (send(socket_fd, packet, request->size, MSG_NOSIGNAL) != (ssize_t)request->size) {
        int saved = errno ? errno : EIO;
        close(socket_fd);
        errno = saved;
        return -1;
    }

    unsigned char reply[sizeof(struct xcm_response) + XCM_MAX_MESSAGE_SIZE];
    ssize_t received = recv(socket_fd, reply, sizeof(reply), MSG_TRUNC);
    int saved = errno;
    close(socket_fd);
    if (received < (ssize_t)sizeof(*response) ||
        received > (ssize_t)sizeof(reply)) {
        errno = received < 0 ? saved : EPROTO;
        return -1;
    }
    memcpy(response, reply, sizeof(*response));
    if (response->version != XCM_PROTOCOL_VERSION ||
        response->size != (uint32_t)received ||
        response->payload_size != (uint32_t)received - sizeof(*response) ||
        response->request_id != request->request_id ||
        response->payload_size > response_capacity) {
        errno = EPROTO;
        return -1;
    }
    if (response->payload_size != 0)
        memcpy(response_payload, reply + sizeof(*response), response->payload_size);
    return 0;
}

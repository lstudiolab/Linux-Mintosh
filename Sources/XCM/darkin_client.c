#define _GNU_SOURCE
#include "include/xcm/sean_client.h"
#include <sean/protocol.h>

#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <unistd.h>

int xcm_darkin_system_info(uint64_t request_id, char *buffer,
                           size_t capacity, uint32_t *payload_size)
{
    if (request_id == 0 || buffer == NULL || capacity == 0 ||
        payload_size == NULL || capacity > SEAN_MAX_PACKET_SIZE) {
        errno = EINVAL;
        return -1;
    }

    int socket_fd = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0);
    if (socket_fd < 0) return -1;
    struct timeval timeout = { .tv_sec = 2, .tv_usec = 0 };
    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0 ||
        setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
        int saved = errno;
        close(socket_fd);
        errno = saved;
        return -1;
    }

    struct sockaddr_un address;
    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    memcpy(address.sun_path, SEAN_SYSTEM_SOCKET, sizeof(SEAN_SYSTEM_SOCKET));
    if (connect(socket_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        int saved = errno;
        close(socket_fd);
        errno = saved;
        return -1;
    }

    struct sean_request request = {
        sizeof(request), SEAN_PROTOCOL_VERSION,
        SEAN_OP_READ_SYSTEM_INFO, request_id
    };
    if (send(socket_fd, &request, sizeof(request), MSG_NOSIGNAL) !=
        (ssize_t)sizeof(request)) {
        int saved = errno ? errno : EIO;
        close(socket_fd);
        errno = saved;
        return -1;
    }

    unsigned char packet[sizeof(struct sean_response) + SEAN_MAX_PACKET_SIZE];
    ssize_t received = recv(socket_fd, packet, sizeof(packet), MSG_TRUNC);
    int saved = errno;
    close(socket_fd);
    if (received < (ssize_t)sizeof(struct sean_response) ||
        received > (ssize_t)sizeof(packet)) {
        errno = received < 0 ? saved : EPROTO;
        return -1;
    }

    struct sean_response response;
    memcpy(&response, packet, sizeof(response));
    if (response.version != SEAN_PROTOCOL_VERSION ||
        response.status != SEAN_STATUS_OK ||
        response.request_id != request_id ||
        response.reserved != 0 ||
        response.size != (uint32_t)received ||
        response.payload_size != (uint32_t)received - sizeof(response) ||
        response.payload_size > capacity) {
        errno = response.status == SEAN_STATUS_DENIED ? EACCES : EPROTO;
        return -1;
    }
    memcpy(buffer, packet + sizeof(response), response.payload_size);
    *payload_size = response.payload_size;
    return 0;
}

#define _GNU_SOURCE
#include <xcu/xcu.h>

#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int xcu_port_pair(int ports[2])
{
    if (ports == NULL) { errno = EINVAL; return -1; }
    return socketpair(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0, ports);
}

ssize_t xcu_port_send(int port, const void *message, size_t size, int pass_fd)
{
    if (port < 0 || (size != 0 && message == NULL) ||
        size > XCU_PORT_MAX_MESSAGE) {
        errno = EINVAL;
        return -1;
    }
    struct iovec vector = { .iov_base = (void *)message, .iov_len = size };
    struct msghdr header;
    memset(&header, 0, sizeof(header));
    header.msg_iov = &vector;
    header.msg_iovlen = 1;

    char control[CMSG_SPACE(sizeof(int))];
    if (pass_fd >= 0) {
        memset(control, 0, sizeof(control));
        header.msg_control = control;
        header.msg_controllen = sizeof(control);
        struct cmsghdr *item = CMSG_FIRSTHDR(&header);
        item->cmsg_level = SOL_SOCKET;
        item->cmsg_type = SCM_RIGHTS;
        item->cmsg_len = CMSG_LEN(sizeof(int));
        memcpy(CMSG_DATA(item), &pass_fd, sizeof(pass_fd));
    }
    return sendmsg(port, &header, MSG_NOSIGNAL);
}

ssize_t xcu_port_receive(int port, void *buffer, size_t capacity, int *receive_fd)
{
    if (port < 0 || buffer == NULL || capacity == 0 ||
        capacity > XCU_PORT_MAX_MESSAGE) {
        errno = EINVAL;
        return -1;
    }
    if (receive_fd != NULL) *receive_fd = -1;

    struct iovec vector = { .iov_base = buffer, .iov_len = capacity };
    char control[CMSG_SPACE(sizeof(int) * 4U)];
    struct msghdr header;
    memset(&header, 0, sizeof(header));
    memset(control, 0, sizeof(control));
    header.msg_iov = &vector;
    header.msg_iovlen = 1;
    header.msg_control = control;
    header.msg_controllen = sizeof(control);

    ssize_t received = recvmsg(port, &header, MSG_CMSG_CLOEXEC);
    if (received < 0) return -1;

    int found_fd = -1;
    bool invalid_control = false;
    for (struct cmsghdr *item = CMSG_FIRSTHDR(&header); item != NULL;
         item = CMSG_NXTHDR(&header, item)) {
        if (item->cmsg_level != SOL_SOCKET || item->cmsg_type != SCM_RIGHTS ||
            item->cmsg_len < CMSG_LEN(sizeof(int))) {
            invalid_control = true;
            continue;
        }
        size_t bytes = item->cmsg_len - CMSG_LEN(0);
        if (bytes % sizeof(int) != 0) invalid_control = true;
        size_t count = bytes / sizeof(int);
        int *descriptors = (int *)CMSG_DATA(item);
        for (size_t i = 0; i < count; ++i) {
            if (found_fd < 0) found_fd = descriptors[i];
            else {
                close(descriptors[i]);
                invalid_control = true;
            }
        }
    }

    if ((header.msg_flags & (MSG_TRUNC | MSG_CTRUNC)) != 0 || invalid_control) {
        if (found_fd >= 0) close(found_fd);
        errno = (header.msg_flags & (MSG_TRUNC | MSG_CTRUNC)) != 0
            ? EMSGSIZE : EPROTO;
        return -1;
    }
    if (found_fd >= 0) {
        if (receive_fd == NULL) {
            close(found_fd);
            errno = EPROTO;
            return -1;
        }
        *receive_fd = found_fd;
    }
    return received;
}

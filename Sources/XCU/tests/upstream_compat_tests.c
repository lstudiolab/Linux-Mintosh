#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

#ifndef MFD_ALLOW_SEALING
#define MFD_ALLOW_SEALING 0x0002U
#endif
#ifndef F_ADD_SEALS
#define F_ADD_SEALS 1033
#define F_GET_SEALS 1034
#endif
#ifndef F_SEAL_SEAL
#define F_SEAL_SEAL 0x0001
#define F_SEAL_SHRINK 0x0002
#define F_SEAL_GROW 0x0004
#define F_SEAL_WRITE 0x0008
#endif

static int failures;

#define CHECK(condition, label) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s (%s:%d): %s\n", label, __FILE__, __LINE__, strerror(errno)); \
        ++failures; \
    } else { \
        printf("PASS: %s\n", label); \
    } \
} while (0)

static void test_seqpacket_descriptor_passing(void) {
    int sockets[2] = {-1, -1};
    int event = -1;
    int received = -1;
    char byte = 'X';
    char output = 0;
    struct iovec send_iov = {.iov_base = &byte, .iov_len = sizeof(byte)};
    struct iovec recv_iov = {.iov_base = &output, .iov_len = sizeof(output)};
    char control[CMSG_SPACE(sizeof(int))];
    struct msghdr send_message = {0};
    struct msghdr recv_message = {0};

    CHECK(socketpair(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0, sockets) == 0,
          "Mach-style local message port uses Unix SOCK_SEQPACKET");
    if (sockets[0] < 0 || sockets[1] < 0) return;

    event = eventfd(0, EFD_CLOEXEC);
    CHECK(event >= 0, "create transferable event descriptor");
    if (event < 0) goto done;

    send_message.msg_iov = &send_iov;
    send_message.msg_iovlen = 1;
    send_message.msg_control = control;
    send_message.msg_controllen = sizeof(control);
    struct cmsghdr *send_header = CMSG_FIRSTHDR(&send_message);
    if (send_header == NULL) goto done;
    send_header->cmsg_level = SOL_SOCKET;
    send_header->cmsg_type = SCM_RIGHTS;
    send_header->cmsg_len = CMSG_LEN(sizeof(int));
    memcpy(CMSG_DATA(send_header), &event, sizeof(event));
    CHECK(sendmsg(sockets[0], &send_message, 0) == 1,
          "send typed message with SCM_RIGHTS descriptor");

    recv_message.msg_iov = &recv_iov;
    recv_message.msg_iovlen = 1;
    recv_message.msg_control = control;
    recv_message.msg_controllen = sizeof(control);
    CHECK(recvmsg(sockets[1], &recv_message, 0) == 1 && output == byte,
          "receive message and attached descriptor");
    struct cmsghdr *recv_header = CMSG_FIRSTHDR(&recv_message);
    if (recv_header != NULL && recv_header->cmsg_level == SOL_SOCKET &&
        recv_header->cmsg_type == SCM_RIGHTS &&
        recv_header->cmsg_len >= CMSG_LEN(sizeof(int))) {
        memcpy(&received, CMSG_DATA(recv_header), sizeof(received));
    }
    CHECK(received >= 0 && fcntl(received, F_GETFD) >= 0,
          "received descriptor is valid in peer process");

done:
    if (received >= 0) close(received);
    if (event >= 0) close(event);
    close(sockets[0]);
    close(sockets[1]);
}

static void test_sealed_memory_object(void) {
    int fd = (int)syscall(SYS_memfd_create, "xcu-memory-test", MFD_ALLOW_SEALING | 0x0001U);
    CHECK(fd >= 0, "create Mach-style memory object with memfd");
    if (fd < 0) return;

    CHECK(ftruncate(fd, 4096) == 0, "size memory object");
    void *mapping = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    CHECK(mapping != MAP_FAILED, "map shared memory object");
    if (mapping != MAP_FAILED) {
        memset(mapping, 0x5a, 4096);
        CHECK(munmap(mapping, 4096) == 0, "unmap before sealing memory object");
    }

    int seals = F_SEAL_WRITE | F_SEAL_GROW | F_SEAL_SHRINK | F_SEAL_SEAL;
    CHECK(fcntl(fd, F_ADD_SEALS, seals) == 0, "seal memory object against mutation and resizing");
    CHECK((fcntl(fd, F_GET_SEALS) & seals) == seals, "verify immutable memory-object seals");
    errno = 0;
    CHECK(ftruncate(fd, 8192) < 0 && errno == EPERM,
          "sealed memory object rejects resize");
    close(fd);
}

static void test_event_delivery(void) {
    int epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    int event_fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    CHECK(epoll_fd >= 0 && event_fd >= 0, "create Mach-style event queue and event source");
    if (epoll_fd < 0 || event_fd < 0) goto done;

    struct epoll_event registration = {.events = EPOLLIN, .data.u64 = 0x584355U};
    CHECK(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, event_fd, &registration) == 0,
          "register event source");
    uint64_t one = 1;
    CHECK(write(event_fd, &one, sizeof(one)) == (ssize_t)sizeof(one),
          "signal event source");
    struct epoll_event ready;
    int count = epoll_wait(epoll_fd, &ready, 1, 1000);
    CHECK(count == 1 && ready.data.u64 == 0x584355U,
          "deliver event through XCU event queue");
done:
    if (event_fd >= 0) close(event_fd);
    if (epoll_fd >= 0) close(epoll_fd);
}

static void test_posix_and_system_v_queues(void) {
    char name[64];
    (void)snprintf(name, sizeof(name), "/lenix-xcu-%ld", (long)getpid());
    struct mq_attr attributes = {.mq_maxmsg = 4, .mq_msgsize = 64};
    mqd_t queue = mq_open(name, O_CREAT | O_EXCL | O_RDWR | O_CLOEXEC, 0600, &attributes);
    CHECK(queue != (mqd_t)-1, "create POSIX message queue");
    if (queue != (mqd_t)-1) {
        const char message[] = "posix-message";
        char received[64] = {0};
        CHECK(mq_send(queue, message, sizeof(message), 3) == 0,
              "send POSIX message queue item");
        CHECK(mq_receive(queue, received, sizeof(received), NULL) == (ssize_t)sizeof(message) &&
              memcmp(received, message, sizeof(message)) == 0,
              "receive POSIX message queue item");
        mq_close(queue);
        mq_unlink(name);
    }

    struct {
        long type;
        char text[32];
    } message = {.type = 1, .text = "sysv-message"}, received = {0};
    int queue_id = msgget(IPC_PRIVATE, IPC_CREAT | 0600);
    CHECK(queue_id >= 0, "create System V message queue");
    if (queue_id >= 0) {
        CHECK(msgsnd(queue_id, &message, sizeof(message.text), 0) == 0,
              "send System V message");
        CHECK(msgrcv(queue_id, &received, sizeof(received.text), 1, 0) ==
              (ssize_t)sizeof(message.text) &&
              strcmp(received.text, message.text) == 0,
              "receive System V message");
        CHECK(msgctl(queue_id, IPC_RMID, NULL) == 0, "remove System V message queue");
    }
}

static void test_pidfd_task_identity(void) {
#ifdef SYS_pidfd_open
    int pidfd = (int)syscall(SYS_pidfd_open, getpid(), 0);
    CHECK(pidfd >= 0, "obtain Linux pidfd for Mach-style task identity");
    if (pidfd >= 0) close(pidfd);
#else
    fprintf(stderr, "SKIP: pidfd_open syscall number unavailable at build time\n");
#endif
}

int main(void) {
    test_seqpacket_descriptor_passing();
    test_sealed_memory_object();
    test_event_delivery();
    test_posix_and_system_v_queues();
    test_pidfd_task_identity();
    if (failures != 0) {
        fprintf(stderr, "%d XCU upstream-compatibility checks failed\n", failures);
        return EXIT_FAILURE;
    }
    puts("All XCU Linux/BSD-POSIX/Mach-concept compatibility checks passed.");
    return EXIT_SUCCESS;
}

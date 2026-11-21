#define _GNU_SOURCE
#include "uevent.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <unistd.h>

static volatile sig_atomic_t watching = 1;
static void stop_watching(int signal_number) { (void)signal_number; watching = 0; }

static void print_json(const char *value) {
    putchar('"');
    for (const unsigned char *p = (const unsigned char *)value; *p; ++p) {
        if (*p == '"' || *p == '\\') { putchar('\\'); putchar(*p); }
        else if (*p < 0x20) printf("\\u%04x", *p);
        else putchar(*p);
    }
    putchar('"');
}
int ioplus_watch_uevents(void) {
    int descriptor = socket(AF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC, NETLINK_KOBJECT_UEVENT);
    if (descriptor < 0) { perror("ioplus: uevent socket"); return 1; }
    int receive_buffer = 256 * 1024;
    (void)setsockopt(descriptor, SOL_SOCKET, SO_RCVBUF, &receive_buffer, sizeof receive_buffer);
    struct sockaddr_nl address = {.nl_family = AF_NETLINK, .nl_pid = (unsigned int)getpid(), .nl_groups = 1};
    if (bind(descriptor, (struct sockaddr *)&address, sizeof address) < 0) {
        perror("ioplus: bind uevent socket"); close(descriptor); return 1;
    }
    struct sigaction action = {0};
    action.sa_handler = stop_watching;
    sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
    unsigned long long dropped = 0;
    while (watching) {
        char buffer[8192];
        struct iovec vector = {.iov_base = buffer, .iov_len = sizeof buffer};
        struct sockaddr_nl sender = {0};
        struct msghdr message = {
            .msg_name = &sender, .msg_namelen = sizeof sender,
            .msg_iov = &vector, .msg_iovlen = 1
        };
        ssize_t length = recvmsg(descriptor, &message, 0);
        if (length < 0) {
            if (errno == EINTR) continue;
            perror("ioplus: receive uevent"); close(descriptor); return 1;
        }
        if ((message.msg_flags & MSG_TRUNC) || sender.nl_pid != 0) { ++dropped; continue; }
        IOPLUSUEvent event;
        if (ioplus_parse_uevent(buffer, (size_t)length, &event) < 0) { ++dropped; continue; }
        fputs("{\"action\":", stdout); print_json(event.action);
        fputs(",\"devpath\":", stdout); print_json(event.devpath);
        fputs(",\"subsystem\":", stdout); print_json(event.subsystem);
        fputs(",\"devtype\":", stdout); print_json(event.devtype);
        fputs(",\"driver\":", stdout); print_json(event.driver);
        fputs(",\"product\":", stdout); print_json(event.product);
        fputs(",\"interface\":", stdout); print_json(event.interface_name);
        fputs(",\"sequence\":", stdout); print_json(event.sequence);
        printf(",\"dropped_local\":%llu}\n", dropped);
        fflush(stdout);
    }
    close(descriptor);
    return 0;
}

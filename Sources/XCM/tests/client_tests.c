#define _GNU_SOURCE
#include "../include/xcm/client.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    char path[] = "/tmp/mintosh-xcm-test-XXXXXX";
    int temporary = mkstemp(path);
    assert(temporary >= 0);
    close(temporary);
    unlink(path);

    int server = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0);
    assert(server >= 0);
    struct sockaddr_un address;
    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    memcpy(address.sun_path, path, strlen(path) + 1U);
    assert(bind(server, (struct sockaddr *)&address, sizeof(address)) == 0);
    assert(listen(server, 1) == 0);

    pid_t child = fork();
    assert(child >= 0);
    if (child == 0) {
        int peer = accept4(server, NULL, NULL, SOCK_CLOEXEC);
        if (peer < 0) _exit(10);
        unsigned char request_packet[XCM_MAX_MESSAGE_SIZE];
        ssize_t received = recv(peer, request_packet, sizeof(request_packet), 0);
        if (received != (ssize_t)(sizeof(struct xcm_request) + 4U)) _exit(11);
        struct xcm_request request;
        memcpy(&request, request_packet, sizeof(request));
        if (request.request_id != 77 || request.payload_type != XCM_PAYLOAD_UI_TOKENS ||
            memcmp(request_packet + sizeof(request), "USI!", 4) != 0) _exit(12);

        const char reply_payload[] = "routed";
        struct xcm_response response = {
            (uint32_t)(sizeof(response) + sizeof(reply_payload) - 1U),
            XCM_PROTOCOL_VERSION,
            XCM_RESPONSE_OK,
            request.request_id,
            XCM_RESPONSE_APP_EVENT,
            (uint32_t)(sizeof(reply_payload) - 1U)
        };
        unsigned char reply[sizeof(response) + sizeof(reply_payload) - 1U];
        memcpy(reply, &response, sizeof(response));
        memcpy(reply + sizeof(response), reply_payload, sizeof(reply_payload) - 1U);
        ssize_t sent = send(peer, reply, sizeof(reply), 0);
        close(peer);
        close(server);
        _exit(sent == (ssize_t)sizeof(reply) ? 0 : 13);
    }

    close(server);
    const char payload[] = "USI!";
    struct xcm_request request = {
        (uint32_t)(sizeof(request) + sizeof(payload) - 1U),
        XCM_PROTOCOL_VERSION,
        XCM_OP_PRESENT_TOKENS,
        0,
        XCM_TARGET_PRESENTATION,
        0,
        XCM_PAYLOAD_UI_TOKENS,
        (uint32_t)(sizeof(payload) - 1U),
        77
    };
    struct xcm_response response;
    char response_payload[16] = {0};
    assert(xcm_call(path, &request, payload, &response,
                    response_payload, sizeof(response_payload)) == 0);
    assert(response.status == XCM_RESPONSE_OK);
    assert(response.payload_type == XCM_RESPONSE_APP_EVENT);
    assert(response.payload_size == 6);
    assert(memcmp(response_payload, "routed", 6) == 0);

    int child_status = 0;
    assert(waitpid(child, &child_status, 0) == child);
    assert(WIFEXITED(child_status) && WEXITSTATUS(child_status) == 0);
    unlink(path);
    puts("XCM client transport tests passed.");
    return 0;
}

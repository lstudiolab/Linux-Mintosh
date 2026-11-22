#include "../include/xcm/protocol.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static struct xcm_request make_request(uint16_t operation, uint32_t target,
                                       uint32_t payload_type, uint32_t payload_size)
{
    struct xcm_request request;
    memset(&request, 0, sizeof(request));
    request.size = (uint32_t)sizeof(request) + payload_size;
    request.version = XCM_PROTOCOL_VERSION;
    request.operation = operation;
    request.target = target;
    request.payload_type = payload_type;
    request.payload_size = payload_size;
    return request;
}

int main(void)
{
    struct xcm_policy_context choca = {
        XCM_PEER_CHOCA,
        XCM_CAP_PRESENT | XCM_CAP_READ_SYSTEM | XCM_CAP_READ_USER_DATA |
            XCM_CAP_WRITE_USER_DATA | XCM_CAP_DELETE_USER_DATA |
            XCM_CAP_FOUNDATION_UPDATE,
        4096,
        true,
        true
    };
    struct xcm_request request = make_request(
        XCM_OP_PRESENT_TOKENS, XCM_TARGET_PRESENTATION, XCM_PAYLOAD_UI_TOKENS, 8);
    assert(xcm_policy_authorize(&request, &choca) == XCM_POLICY_ALLOW);

    request = make_request(XCM_OP_READ_SYSTEM_INFO, XCM_TARGET_SYSTEM_INFO,
                           XCM_PAYLOAD_NONE, 0);
    assert(xcm_policy_authorize(&request, &choca) == XCM_POLICY_ALLOW);

    request = make_request(XCM_OP_DELETE_USER_DATA, XCM_TARGET_USER_DATA,
                           XCM_PAYLOAD_NONE, 0);
    request.object_id = 42;
    assert(xcm_policy_authorize(&request, &choca) == XCM_POLICY_CONFIRMATION_REQUIRED);
    request.flags = XCM_FLAG_USER_CONFIRMED;
    assert(xcm_policy_authorize(&request, &choca) == XCM_POLICY_ALLOW);

    /* Writes require typed data; deletes must not smuggle a payload. */
    request = make_request(XCM_OP_WRITE_USER_DATA, XCM_TARGET_USER_DATA,
                           XCM_PAYLOAD_NONE, 0);
    request.object_id = 99;
    assert(xcm_policy_authorize(&request, &choca) == XCM_POLICY_BAD_PAYLOAD);
    request = make_request(XCM_OP_WRITE_USER_DATA, XCM_TARGET_USER_DATA,
                           XCM_PAYLOAD_TYPED_DATA, 8);
    request.object_id = 99;
    assert(xcm_policy_authorize(&request, &choca) == XCM_POLICY_ALLOW);
    request = make_request(XCM_OP_DELETE_USER_DATA, XCM_TARGET_USER_DATA,
                           XCM_PAYLOAD_TYPED_DATA, 8);
    request.object_id = 99;
    request.flags = XCM_FLAG_USER_CONFIRMED;
    assert(xcm_policy_authorize(&request, &choca) == XCM_POLICY_BAD_PAYLOAD);

    /* Choca cannot mutate Darkin, even when its context has every capability. */
    request = make_request(XCM_OP_UPDATE_DARKIN, XCM_TARGET_DARKIN_CORE,
                           XCM_PAYLOAD_TYPED_DATA, 8);
    assert(xcm_policy_authorize(&request, &choca) == XCM_POLICY_PROTECTED_TARGET);
    request = make_request(XCM_OP_DELETE_USER_DATA, XCM_TARGET_SECURITY_POLICY,
                           XCM_PAYLOAD_NONE, 0);
    request.object_id = 1;
    assert(xcm_policy_authorize(&request, &choca) == XCM_POLICY_PROTECTED_TARGET);
    request = make_request(XCM_OP_EXECUTE_RAW, XCM_TARGET_USER_DATA,
                           XCM_PAYLOAD_NONE, 0);
    assert(xcm_policy_authorize(&request, &choca) == XCM_POLICY_PROTECTED_TARGET);

    /* UI tokens cannot be smuggled into a system operation. */
    request = make_request(XCM_OP_READ_SYSTEM_INFO, XCM_TARGET_SYSTEM_INFO,
                           XCM_PAYLOAD_UI_TOKENS, 8);
    assert(xcm_policy_authorize(&request, &choca) == XCM_POLICY_FORBIDDEN_PAYLOAD);

    request = make_request(XCM_OP_READ_SYSTEM_INFO, XCM_TARGET_SYSTEM_INFO,
                           XCM_PAYLOAD_NONE, 0);
    request.version++;
    assert(xcm_policy_authorize(&request, &choca) == XCM_POLICY_BAD_VERSION);
    request = make_request(XCM_OP_READ_SYSTEM_INFO, XCM_TARGET_SYSTEM_INFO,
                           XCM_PAYLOAD_NONE, 0);
    request.payload_size = 5000;
    request.size += 5000;
    assert(xcm_policy_authorize(&request, &choca) == XCM_POLICY_BAD_SIZE);

    choca.transport_authenticated = false;
    assert(xcm_policy_authorize(&request, &choca) == XCM_POLICY_UNAUTHENTICATED_PEER);

    struct xcm_policy_context updater = {
        XCM_PEER_UPDATE_SERVICE, XCM_CAP_FOUNDATION_UPDATE, 4096, true, false
    };
    request = make_request(XCM_OP_UPDATE_DARKIN, XCM_TARGET_DARKIN_CORE,
                           XCM_PAYLOAD_TYPED_DATA, 8);
    assert(xcm_policy_authorize(&request, &updater) == XCM_POLICY_UNVERIFIED_UPDATE);
    updater.verified_update_bundle = true;
    assert(xcm_policy_authorize(&request, &updater) == XCM_POLICY_ALLOW);

    puts("XCM policy tests passed.");
    return 0;
}

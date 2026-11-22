#include "xcm_policy.h"
#include <assert.h>
#include <stdio.h>

static xcm_peer choca(uint64_t capabilities) {
    xcm_peer peer = { XCM_PEER_CHOCA, capabilities, true, true, true };
    return peer;
}

static xcm_request request(xcm_operation operation, xcm_target target) {
    xcm_request value = { XCM_PROTOCOL_VERSION, 1, operation, target, 0, 1, 0, false };
    return value;
}

int main(void) {
    xcm_peer peer = choca(UINT64_MAX);
    xcm_request value = request(XCM_OP_DELETE_FOUNDATION, XCM_TARGET_DARKIN_FOUNDATION);
    assert(xcm_policy_evaluate(&peer, &value) == XCM_DENY_PROTECTED_FOUNDATION);

    value = request(XCM_OP_UPDATE_FOUNDATION, XCM_TARGET_DARKIN_FOUNDATION);
    assert(xcm_policy_evaluate(&peer, &value) == XCM_DENY_PROTECTED_FOUNDATION);

    value = request(XCM_OP_PRESENT_TOKENS, XCM_TARGET_PRESENTATION);
    assert(xcm_policy_evaluate(&peer, &value) == XCM_DECISION_ALLOW);

    value = request(XCM_OP_DELETE_USER_DATA, XCM_TARGET_USER_DATA);
    assert(xcm_policy_evaluate(&peer, &value) == XCM_DENY_CONFIRMATION_REQUIRED);
    value.explicit_confirmation = true;
    assert(xcm_policy_evaluate(&peer, &value) == XCM_DECISION_ALLOW);
    value.resource_handle = 0;
    assert(xcm_policy_evaluate(&peer, &value) == XCM_DENY_UNSCOPED_RESOURCE);

    value = request(XCM_OP_INVOKE_SERVICE, XCM_TARGET_SYSTEM_SERVICE);
    value.service_id = 999;
    assert(xcm_policy_evaluate(&peer, &value) == XCM_DENY_UNKNOWN_SERVICE);

    value = request(XCM_OP_READ_SYSTEM_INFO, XCM_TARGET_SYSTEM_INFO);
    value.version++;
    assert(xcm_policy_evaluate(&peer, &value) == XCM_DENY_INVALID_MESSAGE);
    value.version = XCM_PROTOCOL_VERSION;
    value.payload_size = XCM_MAX_PAYLOAD_SIZE + 1;
    assert(xcm_policy_evaluate(&peer, &value) == XCM_DENY_INVALID_MESSAGE);

    peer.authenticated = false;
    assert(xcm_policy_evaluate(&peer, &value) == XCM_DENY_UNAUTHENTICATED);

    peer = (xcm_peer){ XCM_PEER_UPDATE_SERVICE, XCM_CAP_FOUNDATION_UPDATE, true, true, false };
    value = request(XCM_OP_UPDATE_FOUNDATION, XCM_TARGET_DARKIN_FOUNDATION);
    assert(xcm_policy_evaluate(&peer, &value) == XCM_DENY_UNVERIFIED_UPDATE);
    peer.verified_update_bundle = true;
    assert(xcm_policy_evaluate(&peer, &value) == XCM_DECISION_ALLOW);

    value = request((xcm_operation)999, XCM_TARGET_SYSTEM_INFO);
    assert(xcm_policy_evaluate(&peer, &value) == XCM_DENY_UNKNOWN_OPERATION);

    puts("XCM policy tests passed.");
    return 0;
}

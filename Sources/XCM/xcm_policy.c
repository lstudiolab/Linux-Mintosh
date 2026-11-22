#include "xcm_policy.h"

static bool has_capability(const xcm_peer *peer, uint64_t capability) {
    return (peer->capabilities & capability) == capability;
}

static bool known_service(uint32_t service_id) {
    return service_id == XCM_SERVICE_SETTINGS ||
           service_id == XCM_SERVICE_POWER ||
           service_id == XCM_SERVICE_NETWORK ||
           service_id == XCM_SERVICE_STORAGE;
}

xcm_decision xcm_policy_evaluate(const xcm_peer *peer, const xcm_request *request) {
    if (peer == NULL || request == NULL || !peer->authenticated || !peer->local_session)
        return XCM_DENY_UNAUTHENTICATED;

    if (request->version != XCM_PROTOCOL_VERSION || request->request_id == 0 ||
        request->payload_size > XCM_MAX_PAYLOAD_SIZE)
        return XCM_DENY_INVALID_MESSAGE;

    /* Choca can never mutate Darkin, even if a request claims every capability. */
    if (peer->kind == XCM_PEER_CHOCA && request->target == XCM_TARGET_DARKIN_FOUNDATION)
        return XCM_DENY_PROTECTED_FOUNDATION;

    switch (request->operation) {
    case XCM_OP_PRESENT_TOKENS:
        if (peer->kind != XCM_PEER_CHOCA || request->target != XCM_TARGET_PRESENTATION)
            return XCM_DENY_WRONG_TARGET;
        return has_capability(peer, XCM_CAP_PRESENT) ? XCM_DECISION_ALLOW : XCM_DENY_MISSING_CAPABILITY;

    case XCM_OP_READ_SYSTEM_INFO:
        if (request->target != XCM_TARGET_SYSTEM_INFO ||
            (peer->kind != XCM_PEER_CHOCA && peer->kind != XCM_PEER_DARKIN_SERVICE))
            return XCM_DENY_WRONG_TARGET;
        return has_capability(peer, XCM_CAP_SYSTEM_INFO_READ) ? XCM_DECISION_ALLOW : XCM_DENY_MISSING_CAPABILITY;

    case XCM_OP_READ_USER_DATA:
    case XCM_OP_WRITE_USER_DATA:
    case XCM_OP_DELETE_USER_DATA: {
        uint64_t required;
        if (request->target != XCM_TARGET_USER_DATA || peer->kind != XCM_PEER_CHOCA)
            return XCM_DENY_WRONG_TARGET;
        if (request->resource_handle == 0)
            return XCM_DENY_UNSCOPED_RESOURCE;
        required = request->operation == XCM_OP_READ_USER_DATA ? XCM_CAP_USER_DATA_READ :
                   request->operation == XCM_OP_WRITE_USER_DATA ? XCM_CAP_USER_DATA_WRITE :
                   XCM_CAP_USER_DATA_DELETE;
        if (!has_capability(peer, required))
            return XCM_DENY_MISSING_CAPABILITY;
        if (request->operation == XCM_OP_DELETE_USER_DATA && !request->explicit_confirmation)
            return XCM_DENY_CONFIRMATION_REQUIRED;
        return XCM_DECISION_ALLOW;
    }

    case XCM_OP_INVOKE_SERVICE:
        if (request->target != XCM_TARGET_SYSTEM_SERVICE || peer->kind != XCM_PEER_CHOCA)
            return XCM_DENY_WRONG_TARGET;
        if (!known_service(request->service_id))
            return XCM_DENY_UNKNOWN_SERVICE;
        return has_capability(peer, XCM_CAP_SERVICE_INVOKE) ? XCM_DECISION_ALLOW : XCM_DENY_MISSING_CAPABILITY;

    case XCM_OP_UPDATE_FOUNDATION:
        if (request->target != XCM_TARGET_DARKIN_FOUNDATION || peer->kind != XCM_PEER_UPDATE_SERVICE)
            return XCM_DENY_PROTECTED_FOUNDATION;
        if (!has_capability(peer, XCM_CAP_FOUNDATION_UPDATE))
            return XCM_DENY_MISSING_CAPABILITY;
        return peer->verified_update_bundle ? XCM_DECISION_ALLOW : XCM_DENY_UNVERIFIED_UPDATE;

    case XCM_OP_DELETE_FOUNDATION:
    case XCM_OP_REPLACE_FOUNDATION:
        return XCM_DENY_PROTECTED_FOUNDATION;

    default:
        return XCM_DENY_UNKNOWN_OPERATION;
    }
}

const char *xcm_decision_name(xcm_decision decision) {
    static const char *const names[] = {
        "allow", "unauthenticated", "invalid-message", "unknown-operation",
        "wrong-target", "missing-capability", "unscoped-resource",
        "confirmation-required", "protected-foundation", "unverified-update",
        "unknown-service"
    };
    if ((unsigned)decision >= sizeof(names) / sizeof(names[0]))
        return "invalid-decision";
    return names[decision];
}

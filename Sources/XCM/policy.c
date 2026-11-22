#include "include/xcm/protocol.h"
#include <stddef.h>

#define XCM_KNOWN_FLAGS XCM_FLAG_USER_CONFIRMED
#define XCM_MAX_PAYLOAD_SIZE (XCM_MAX_MESSAGE_SIZE - sizeof(struct xcm_request))

static enum xcm_policy_result require_capability(
    const struct xcm_policy_context *context, uint64_t capability)
{
    return (context->granted_capabilities & capability) == capability
        ? XCM_POLICY_ALLOW : XCM_POLICY_CAPABILITY_DENIED;
}

static bool payload_is(const struct xcm_request *request, uint32_t type)
{
    return request->payload_type == type &&
           ((type == XCM_PAYLOAD_NONE) == (request->payload_size == 0));
}

enum xcm_policy_result xcm_policy_authorize(
    const struct xcm_request *request,
    const struct xcm_policy_context *context)
{
    if (request == NULL || context == NULL)
        return XCM_POLICY_BAD_ARGUMENT;
    if (!context->transport_authenticated ||
        context->authenticated_peer == XCM_PEER_UNKNOWN)
        return XCM_POLICY_UNAUTHENTICATED_PEER;
    if (request->request_id == 0)
        return XCM_POLICY_BAD_ARGUMENT;
    if (request->version != XCM_PROTOCOL_VERSION)
        return XCM_POLICY_BAD_VERSION;
    if (request->size < sizeof(*request) ||
        request->size > XCM_MAX_MESSAGE_SIZE ||
        request->payload_size > XCM_MAX_PAYLOAD_SIZE ||
        request->payload_size > context->maximum_payload_size ||
        request->size != sizeof(*request) + request->payload_size)
        return XCM_POLICY_BAD_SIZE;
    if ((request->flags & ~XCM_KNOWN_FLAGS) != 0)
        return XCM_POLICY_BAD_FLAGS;
    if (request->payload_type > XCM_PAYLOAD_EXECUTABLE)
        return XCM_POLICY_BAD_PAYLOAD;
    if ((request->payload_size == 0) !=
        (request->payload_type == XCM_PAYLOAD_NONE))
        return XCM_POLICY_BAD_PAYLOAD;
    if (request->payload_type == XCM_PAYLOAD_EXECUTABLE)
        return XCM_POLICY_FORBIDDEN_PAYLOAD;

    if (context->authenticated_peer == XCM_PEER_CHOCA) {
        if (request->target == XCM_TARGET_SEAN_CORE ||
            request->target == XCM_TARGET_SECURITY_POLICY ||
            request->target == XCM_TARGET_CHOCA_CORE ||
            request->target == XCM_TARGET_XCM_CORE ||
            request->target == XCM_TARGET_SEAN_BOOT ||
            request->operation == XCM_OP_UPDATE_SEAN ||
            request->operation == XCM_OP_EXECUTE_RAW ||
            request->operation == XCM_OP_CHANGE_SECURITY_POLICY)
            return XCM_POLICY_PROTECTED_TARGET;

        if (request->operation == XCM_OP_PRESENT_TOKENS) {
            if (request->target != XCM_TARGET_PRESENTATION)
                return XCM_POLICY_TARGET_MISMATCH;
            if (!payload_is(request, XCM_PAYLOAD_UI_TOKENS))
                return XCM_POLICY_BAD_PAYLOAD;
            return require_capability(context, XCM_CAP_PRESENT);
        }

        /* UI tokens are presentation data only, never service commands. */
        if (request->payload_type == XCM_PAYLOAD_UI_TOKENS)
            return XCM_POLICY_FORBIDDEN_PAYLOAD;

        switch (request->operation) {
        case XCM_OP_READ_SYSTEM_INFO:
            if (request->target != XCM_TARGET_SYSTEM_INFO)
                return XCM_POLICY_TARGET_MISMATCH;
            if (!payload_is(request, XCM_PAYLOAD_NONE))
                return XCM_POLICY_BAD_PAYLOAD;
            return require_capability(context, XCM_CAP_READ_SYSTEM);

        case XCM_OP_LIST_USER_DATA:
            if (request->target != XCM_TARGET_USER_DATA || request->object_id != 0)
                return XCM_POLICY_TARGET_MISMATCH;
            if (!payload_is(request, XCM_PAYLOAD_NONE))
                return XCM_POLICY_BAD_PAYLOAD;
            return require_capability(context, XCM_CAP_READ_USER_DATA);

        case XCM_OP_READ_USER_DATA:
            if (request->target != XCM_TARGET_USER_DATA || request->object_id == 0)
                return XCM_POLICY_TARGET_MISMATCH;
            if (!payload_is(request, XCM_PAYLOAD_NONE))
                return XCM_POLICY_BAD_PAYLOAD;
            return require_capability(context, XCM_CAP_READ_USER_DATA);

        case XCM_OP_WRITE_USER_DATA:
            if (request->target != XCM_TARGET_USER_DATA || request->object_id == 0)
                return XCM_POLICY_TARGET_MISMATCH;
            if (!payload_is(request, XCM_PAYLOAD_TYPED_DATA))
                return XCM_POLICY_BAD_PAYLOAD;
            return require_capability(context, XCM_CAP_WRITE_USER_DATA);

        case XCM_OP_DELETE_USER_DATA:
            if (request->target != XCM_TARGET_USER_DATA || request->object_id == 0)
                return XCM_POLICY_TARGET_MISMATCH;
            if (!payload_is(request, XCM_PAYLOAD_NONE))
                return XCM_POLICY_BAD_PAYLOAD;
            if ((request->flags & XCM_FLAG_USER_CONFIRMED) == 0 ||
                !context->confirmation_grant_valid ||
                context->confirmation_object_id != request->object_id ||
                context->confirmation_operation != request->operation)
                return XCM_POLICY_CONFIRMATION_REQUIRED;
            return require_capability(context, XCM_CAP_DELETE_USER_DATA);

        default:
            return XCM_POLICY_UNKNOWN_OPERATION;
        }
    }

    if (context->authenticated_peer == XCM_PEER_UPDATE_SERVICE) {
        if (request->operation != XCM_OP_UPDATE_SEAN ||
            request->target != XCM_TARGET_SEAN_CORE ||
            !payload_is(request, XCM_PAYLOAD_TYPED_DATA))
            return XCM_POLICY_PROTECTED_TARGET;
        if (!context->verified_update_bundle)
            return XCM_POLICY_UNVERIFIED_UPDATE;
        return require_capability(context, XCM_CAP_FOUNDATION_UPDATE);
    }

    /* Other Sean services receive no generic authority through this API. */
    return XCM_POLICY_UNAUTHENTICATED_PEER;
}

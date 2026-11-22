#include "include/xcm/protocol.h"
#define XCM_KNOWN_FLAGS XCM_FLAG_USER_CONFIRMED
static enum xcm_policy_result require_capability(const struct xcm_policy_context *c,uint64_t cap)
{
    return (c->granted_capabilities&cap)==cap?XCM_POLICY_ALLOW:XCM_POLICY_CAPABILITY_DENIED;
}
enum xcm_policy_result xcm_policy_authorize(const struct xcm_request *r,const struct xcm_policy_context *c)
{
    if(!r||!c)return XCM_POLICY_BAD_ARGUMENT;
    if(r->version!=XCM_PROTOCOL_VERSION)return XCM_POLICY_BAD_VERSION;
    if(r->size<sizeof(*r)||r->size>XCM_MAX_MESSAGE_SIZE)return XCM_POLICY_BAD_SIZE;
    if(r->flags&~XCM_KNOWN_FLAGS)return XCM_POLICY_BAD_FLAGS;
    if(r->payload_size>c->maximum_payload_size||
       r->payload_size>XCM_MAX_PAYLOAD_SIZE||
       r->size!=sizeof(*r)+r->payload_size)return XCM_POLICY_BAD_SIZE;
    if(r->payload_type==XCM_PAYLOAD_UI_TOKENS||
       r->payload_type==XCM_PAYLOAD_EXECUTABLE)return XCM_POLICY_FORBIDDEN_PAYLOAD;
    if(r->payload_type>XCM_PAYLOAD_EXECUTABLE)return XCM_POLICY_BAD_PAYLOAD;
    if((r->payload_size==0)!=(r->payload_type==XCM_PAYLOAD_NONE))return XCM_POLICY_BAD_PAYLOAD;
    if(c->authenticated_peer!=XCM_PEER_CHOCA)return XCM_POLICY_UNAUTHENTICATED_PEER;
    if(r->target==XCM_TARGET_DARKIN_CORE||
       r->target==XCM_TARGET_SECURITY_POLICY)return XCM_POLICY_PROTECTED_TARGET;
    switch(r->operation){
    case XCM_OP_READ_SYSTEM_INFO:
        if(r->target!=XCM_TARGET_SYSTEM_INFO)return XCM_POLICY_TARGET_MISMATCH;
        return require_capability(c,XCM_CAP_READ_SYSTEM);
    case XCM_OP_LIST_USER_DATA:
    case XCM_OP_READ_USER_DATA:
        if(r->target!=XCM_TARGET_USER_DATA)return XCM_POLICY_TARGET_MISMATCH;
        if(r->operation==XCM_OP_READ_USER_DATA&&!r->object_id)return XCM_POLICY_BAD_ARGUMENT;
        return require_capability(c,XCM_CAP_READ_USER_DATA);
    case XCM_OP_WRITE_USER_DATA:
        if(r->target!=XCM_TARGET_USER_DATA||!r->object_id)return XCM_POLICY_TARGET_MISMATCH;
        return require_capability(c,XCM_CAP_WRITE_USER_DATA);
    case XCM_OP_DELETE_USER_DATA:
        if(r->target!=XCM_TARGET_USER_DATA||!r->object_id)return XCM_POLICY_TARGET_MISMATCH;
        if(!(r->flags&XCM_FLAG_USER_CONFIRMED))return XCM_POLICY_CONFIRMATION_REQUIRED;
        return require_capability(c,XCM_CAP_DELETE_USER_DATA);
    case XCM_OP_UPDATE_DARKIN:
    case XCM_OP_EXECUTE_RAW:
    case XCM_OP_CHANGE_SECURITY_POLICY:
    case XCM_OP_NONE:
    default:
        return XCM_POLICY_UNKNOWN_OPERATION;
    }
}

#include "../include/xcm/protocol.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static struct xcm_request request(uint16_t op,uint32_t target)
{
    struct xcm_request r;
    memset(&r,0,sizeof(r));r.size=sizeof(r);r.version=XCM_PROTOCOL_VERSION;
    r.operation=op;r.target=target;return r;
}
int main(void)
{
    struct xcm_policy_context c={XCM_PEER_CHOCA,XCM_CAP_READ_SYSTEM|XCM_CAP_READ_USER_DATA|XCM_CAP_WRITE_USER_DATA|XCM_CAP_DELETE_USER_DATA,4096};
    struct xcm_request r=request(XCM_OP_READ_SYSTEM_INFO,XCM_TARGET_SYSTEM_INFO);
    assert(xcm_policy_authorize(&r,&c)==XCM_POLICY_ALLOW);
    r=request(XCM_OP_DELETE_USER_DATA,XCM_TARGET_USER_DATA);r.object_id=42;
    assert(xcm_policy_authorize(&r,&c)==XCM_POLICY_CONFIRMATION_REQUIRED);
    r.flags=XCM_FLAG_USER_CONFIRMED;
    assert(xcm_policy_authorize(&r,&c)==XCM_POLICY_ALLOW);
    r.target=XCM_TARGET_DARKIN_CORE;
    assert(xcm_policy_authorize(&r,&c)==XCM_POLICY_PROTECTED_TARGET);
    r=request(XCM_OP_UPDATE_DARKIN,XCM_TARGET_DARKIN_CORE);
    assert(xcm_policy_authorize(&r,&c)==XCM_POLICY_PROTECTED_TARGET);
    r=request(XCM_OP_EXECUTE_RAW,XCM_TARGET_USER_DATA);
    assert(xcm_policy_authorize(&r,&c)==XCM_POLICY_UNKNOWN_OPERATION);
    r=request(XCM_OP_READ_SYSTEM_INFO,XCM_TARGET_SYSTEM_INFO);r.payload_type=XCM_PAYLOAD_UI_TOKENS;r.payload_size=8;r.size+=8;
    assert(xcm_policy_authorize(&r,&c)==XCM_POLICY_FORBIDDEN_PAYLOAD);
    r=request(XCM_OP_READ_SYSTEM_INFO,XCM_TARGET_SYSTEM_INFO);r.version++;
    assert(xcm_policy_authorize(&r,&c)==XCM_POLICY_BAD_VERSION);
    c.authenticated_peer=XCM_PEER_UNKNOWN;r=request(XCM_OP_READ_SYSTEM_INFO,XCM_TARGET_SYSTEM_INFO);
    assert(xcm_policy_authorize(&r,&c)==XCM_POLICY_UNAUTHENTICATED_PEER);
    puts("XCM policy tests passed");
    return 0;
}

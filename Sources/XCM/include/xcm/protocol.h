#ifndef XCM_PROTOCOL_H
#define XCM_PROTOCOL_H

#include <stdbool.h>
#include <stdint.h>

#define XCM_PROTOCOL_VERSION 1U
#define XCM_MAX_MESSAGE_SIZE 65536U

enum xcm_peer {
    XCM_PEER_UNKNOWN = 0,
    XCM_PEER_CHOCA = 1,
    XCM_PEER_DARKIN_SERVICE = 2,
    XCM_PEER_UPDATE_SERVICE = 3
};

enum xcm_target {
    XCM_TARGET_NONE = 0,
    XCM_TARGET_PRESENTATION = 1,
    XCM_TARGET_SYSTEM_INFO = 2,
    XCM_TARGET_USER_DATA = 3,
    XCM_TARGET_DARKIN_CORE = 4,
    XCM_TARGET_SECURITY_POLICY = 5
};

enum xcm_operation {
    XCM_OP_NONE = 0,
    XCM_OP_PRESENT_TOKENS = 1,
    XCM_OP_READ_SYSTEM_INFO = 2,
    XCM_OP_LIST_USER_DATA = 3,
    XCM_OP_READ_USER_DATA = 4,
    XCM_OP_WRITE_USER_DATA = 5,
    XCM_OP_DELETE_USER_DATA = 6,
    XCM_OP_UPDATE_DARKIN = 7,
    XCM_OP_EXECUTE_RAW = 8,
    XCM_OP_CHANGE_SECURITY_POLICY = 9
};

enum xcm_payload_type {
    XCM_PAYLOAD_NONE = 0,
    XCM_PAYLOAD_TYPED_DATA = 1,
    XCM_PAYLOAD_UI_TOKENS = 2,
    XCM_PAYLOAD_EXECUTABLE = 3
};

/* This request flag is only a client hint; it is not proof of confirmation. */
enum xcm_request_flags {
    XCM_FLAG_USER_CONFIRMED = 1U
};

enum xcm_capability {
    XCM_CAP_PRESENT = 1ULL << 0,
    XCM_CAP_READ_SYSTEM = 1ULL << 1,
    XCM_CAP_READ_USER_DATA = 1ULL << 2,
    XCM_CAP_WRITE_USER_DATA = 1ULL << 3,
    XCM_CAP_DELETE_USER_DATA = 1ULL << 4,
    XCM_CAP_FOUNDATION_UPDATE = 1ULL << 5
};

/* Constructed by XCM from authenticated credentials and broker-held state.
 * Never deserialize this structure from Choca. Confirmation is true only
 * after a one-time grant is verified for this exact operation and object.
 */
struct xcm_policy_context {
    enum xcm_peer authenticated_peer;
    uint64_t granted_capabilities;
    uint32_t maximum_payload_size;
    bool transport_authenticated;
    bool verified_update_bundle;
    bool confirmation_grant_valid;
};

/* Fixed-size wire header. Payload bytes immediately follow this header. */
struct xcm_request {
    uint32_t size;
    uint16_t version;
    uint16_t operation;
    uint32_t flags;
    uint32_t target;
    uint64_t object_id;
    uint32_t payload_type;
    uint32_t payload_size;
};

enum xcm_policy_result {
    XCM_POLICY_ALLOW = 0,
    XCM_POLICY_BAD_ARGUMENT,
    XCM_POLICY_BAD_VERSION,
    XCM_POLICY_BAD_SIZE,
    XCM_POLICY_BAD_FLAGS,
    XCM_POLICY_BAD_PAYLOAD,
    XCM_POLICY_UNAUTHENTICATED_PEER,
    XCM_POLICY_UNKNOWN_OPERATION,
    XCM_POLICY_TARGET_MISMATCH,
    XCM_POLICY_CAPABILITY_DENIED,
    XCM_POLICY_PROTECTED_TARGET,
    XCM_POLICY_CONFIRMATION_REQUIRED,
    XCM_POLICY_FORBIDDEN_PAYLOAD,
    XCM_POLICY_UNVERIFIED_UPDATE
};

enum xcm_policy_result xcm_policy_authorize(
    const struct xcm_request *request,
    const struct xcm_policy_context *context
);

#endif

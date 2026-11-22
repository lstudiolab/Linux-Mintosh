#ifndef XCM_POLICY_H
#define XCM_POLICY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define XCM_PROTOCOL_VERSION 1u
#define XCM_MAX_PAYLOAD_SIZE 65536u

typedef enum {
    XCM_PEER_CHOCA = 1,
    XCM_PEER_DARKIN_SERVICE = 2,
    XCM_PEER_UPDATE_SERVICE = 3
} xcm_peer_kind;

typedef enum {
    XCM_TARGET_PRESENTATION = 1,
    XCM_TARGET_SYSTEM_INFO = 2,
    XCM_TARGET_USER_DATA = 3,
    XCM_TARGET_SYSTEM_SERVICE = 4,
    XCM_TARGET_DARKIN_FOUNDATION = 5
} xcm_target;

typedef enum {
    XCM_OP_PRESENT_TOKENS = 1,
    XCM_OP_READ_SYSTEM_INFO = 2,
    XCM_OP_READ_USER_DATA = 3,
    XCM_OP_WRITE_USER_DATA = 4,
    XCM_OP_DELETE_USER_DATA = 5,
    XCM_OP_INVOKE_SERVICE = 6,
    XCM_OP_UPDATE_FOUNDATION = 7,
    XCM_OP_DELETE_FOUNDATION = 8,
    XCM_OP_REPLACE_FOUNDATION = 9
} xcm_operation;

typedef enum {
    XCM_SERVICE_SETTINGS = 1,
    XCM_SERVICE_POWER = 2,
    XCM_SERVICE_NETWORK = 3,
    XCM_SERVICE_STORAGE = 4
} xcm_service_id;

enum {
    XCM_CAP_PRESENT = UINT64_C(1) << 0,
    XCM_CAP_SYSTEM_INFO_READ = UINT64_C(1) << 1,
    XCM_CAP_USER_DATA_READ = UINT64_C(1) << 2,
    XCM_CAP_USER_DATA_WRITE = UINT64_C(1) << 3,
    XCM_CAP_USER_DATA_DELETE = UINT64_C(1) << 4,
    XCM_CAP_SERVICE_INVOKE = UINT64_C(1) << 5,
    XCM_CAP_FOUNDATION_UPDATE = UINT64_C(1) << 6
};

/* Broker-internal metadata; never trust these fields from a wire message. */
typedef struct {
    xcm_peer_kind kind;
    uint64_t capabilities;
    bool authenticated;
    bool local_session;
    bool verified_update_bundle;
} xcm_peer;

typedef struct {
    uint16_t version;
    uint64_t request_id;
    xcm_operation operation;
    xcm_target target;
    uint32_t service_id;
    uint64_t resource_handle;
    uint32_t payload_size;
    bool explicit_confirmation;
} xcm_request;

typedef enum {
    XCM_DECISION_ALLOW = 0,
    XCM_DENY_UNAUTHENTICATED,
    XCM_DENY_INVALID_MESSAGE,
    XCM_DENY_UNKNOWN_OPERATION,
    XCM_DENY_WRONG_TARGET,
    XCM_DENY_MISSING_CAPABILITY,
    XCM_DENY_UNSCOPED_RESOURCE,
    XCM_DENY_CONFIRMATION_REQUIRED,
    XCM_DENY_PROTECTED_FOUNDATION,
    XCM_DENY_UNVERIFIED_UPDATE,
    XCM_DENY_UNKNOWN_SERVICE
} xcm_decision;

/* The transport must derive peer metadata from authenticated credentials and
 * server-held capabilities. Never deserialize xcm_peer from a Choca request.
 */
xcm_decision xcm_policy_evaluate(const xcm_peer *peer, const xcm_request *request);
const char *xcm_decision_name(xcm_decision decision);

#endif

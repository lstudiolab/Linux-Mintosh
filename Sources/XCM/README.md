# XCM — system communication and security broker

XCM is a standalone system component outside Darkin and Choca. It is the only supported communication bridge between Choca and Darkin services.

## Request path

1. Choca sends a versioned typed message to XCM over a local authenticated IPC transport.
2. XCM derives peer identity from authenticated transport credentials and obtains capabilities from broker-held session state. Identity and capabilities are never accepted from the request payload.
3. The policy engine validates message size, version, flags, payload type, target, operation, object scope and required capability.
4. Choca requests targeting Darkin core or security policy are rejected, including update, raw execution and policy-change operations.
5. UI-token payloads are accepted only for the presentation operation. They are never interpreted as privileged commands.
6. User-data deletion requires an object-scoped handle, a delete capability, a client confirmation hint and a one-time confirmation grant verified by XCM for that exact operation and object.
7. Foundation updates are reserved for the authenticated update service and require a verified update bundle plus the foundation-update capability.
8. Darkin independently checks authorization and object ownership before performing an approved operation.

## Implemented policy core

- C17 policy library in `policy.c`.
- Versioned request header and typed payload identifiers in `include/xcm/protocol.h`.
- Deny-by-default handling for unknown operations, invalid messages, unauthenticated peers, protected targets, missing capabilities and unverified updates.
- Per-operation payload rules keep USI tokens confined to presentation and require typed data for user-data writes and foundation updates.
- A Choca-provided confirmation flag is not sufficient; deletion also requires a broker-verified one-time grant.
- Host tests covering Choca foundation-mutation attempts, UI-token separation, scoped user-data deletion, malformed messages and signed-update gating.

Run:

    make -C Sources/XCM test
    make test-xcm

## Remaining before production

This is the policy core, not yet a complete broker/server. Authenticated local IPC transport, capability issuance and revocation, session lifecycle, service dispatch, audit persistence, signature verification, replay protection and kernel-enforced process isolation still need implementation. The broker must construct its policy context from trusted transport and update-verification results; it must never trust those fields from Choca.

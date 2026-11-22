# XCM — system communication and security broker

XCM is a standalone system component outside Darkin and Choca. It is the only supported communication bridge between Choca and Darkin services. Its broker runs as a narrowly confined system service; Choca links only the unprivileged client library.

## Implemented components

- `xcmd.c`: Linux local IPC broker using Unix `SOCK_SEQPACKET` and kernel-provided `SO_PEERCRED` identity.
- `policy.c`: deny-by-default C17 authorization policy.
- `client.c`: Choca-side request/reply client library.
- `include/xcm/protocol.h`: versioned request/response wire envelopes and typed payload identifiers.
- `include/xcm/client.h`: unprivileged client API.
- `Sources/Services/xcm.service`: confined systemd service.
- Host tests for policy decisions and client transport framing.

## Request path

1. Choca connects to `/run/mintosh/xcm.sock` using a local sequenced-packet socket.
2. XCM obtains peer PID, UID and primary GID from the kernel, then verifies the account belongs to `mintosh-services`. Root is not accepted as a Choca client.
3. XCM validates the complete packet size, protocol version, request ID, flags, payload type, target, operation, object scope and capabilities.
4. A bounded recent-request cache rejects duplicate request IDs.
5. Choca receives only presentation and read-only system-information capabilities in this first broker milestone. No user-data write/delete or foundation-update capabilities are issued.
6. System-information requests return a bounded typed response. Presentation requests are policy-checked but return `UNAVAILABLE` until the Choca renderer endpoint is registered.
7. Policy denials and accepted privileged queries are written to the system authentication log.
8. Darkin service implementations must still authorize their own operations; XCM is not a replacement for service-side checks.

## Security properties

- Choca cannot update, delete, replace or execute code in Darkin core or security policy.
- Arbitrary executable payloads are always rejected.
- USI token payloads are accepted only for presentation; they are never treated as commands.
- Local peer identity is kernel-derived, never supplied by the request.
- The broker socket is root-owned, group-restricted and mode `0660`.
- The broker runs with a restricted systemd sandbox and a reduced capability set.
- Unknown operations, malformed packets, unsupported versions and ungranted capabilities fail closed.

## Build, install and test

    make -C Sources/XCM
    make -C Sources/XCM test
    make test-xcm

The root filesystem build installs `xcmd`, `libxcm.a`, public headers and enables `xcm.service`.

## Deliberate implementation limits

This is an operational first broker milestone, not yet the complete production message bus. The following are still required before claiming full XCM completion:

- A registered Choca renderer endpoint and token delivery.
- Darkin service endpoint registration and typed request dispatch.
- Per-application identities/capabilities instead of the initial user-session group policy.
- Durable, bounded audit records with rotation and structured event IDs.
- Cryptographic update-bundle verification and rollback integration.
- A confirmation-grant issuer and atomic single-use grant store before enabling deletion.
- Stronger replay/session semantics and fuzzing of the wire parser.
- Kernel-enforced isolation and a Choca application sandbox.

The current broker intentionally does not issue destructive capabilities or claim to perform operations that have no registered backend.

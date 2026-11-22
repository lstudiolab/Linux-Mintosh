# XCM — system communication and security broker

XCM is a standalone system component outside Darkin and Choca. It is the only supported communication bridge between Choca and Darkin services. Its broker runs as a narrowly confined system service; Choca links only the unprivileged client library.

## Implemented components

- `xcmd.c`: Linux local IPC broker using Unix `SOCK_SEQPACKET` and kernel-provided `SO_PEERCRED` identity.
- `policy.c`: deny-by-default C17 authorization policy.
- `usi_token_validation.c`: strict validator for Choca's presentation-only token stream.
- `client.c`: Choca-side request/reply client library.
- `darkin_client.c`: private XCM-to-Darkin backend client.
- `include/xcm/protocol.h`: versioned request/response wire envelopes and typed payload identifiers.
- `include/xcm/usi_tokens.h`: bounded USI presentation token contract.
- `Sources/Services/xcm.service`: confined systemd service.
- Host tests for policy decisions, client transport framing and USI token validation.

## Request path

1. Choca connects to `/run/mintosh/xcm.sock` using a local sequenced-packet socket.
2. XCM obtains peer PID, UID and primary GID from the kernel, then verifies the process's actual supplementary groups through `/proc/<pid>/status` include `mintosh-services`. Root is not accepted as a Choca client.
3. XCM validates the complete packet size, protocol version, request ID, flags, payload type, target, operation, object scope and capabilities.
4. A bounded recent-request cache keyed by kernel-authenticated UID, PID and request ID rejects duplicate requests.
5. Choca receives only presentation and read-only system-information capabilities in this first broker milestone. No user-data write/delete or foundation-update capabilities are issued.
6. Presentation payloads must pass the USI token validator: bounded token count/depth, known presentation-only token types, valid UTF-8, balanced containers, numeric resource IDs and action IDs. There are no shell, executable, filesystem-path or arbitrary-command tokens.
7. An authorized system-information request is forwarded over a separate root-only socket to the Darkin system service. Darkin authenticates the XCM process and independently validates the internal request before returning typed system data.
8. Valid presentation requests are policy-checked but return `UNAVAILABLE` until the Choca renderer endpoint is registered.
9. Policy denials and accepted system-information queries are written to the system authentication log.

## Security properties

- Choca cannot update, delete, replace or execute code in Darkin core, Darkin boot assets, Choca core, XCM core or security policy.
- Arbitrary executable payloads are always rejected.
- USI tokens are a restricted presentation format, never privileged commands.
- Local peer identity is kernel-derived, never supplied by the request.
- The Choca-facing broker socket is root-owned, group-restricted and mode `0660`.
- The Darkin backend socket is root-owned, mode `0600`, and placed in a root-only runtime directory. Choca cannot connect to it directly.
- Both services run with reduced capability sets and systemd sandboxes.
- Unknown operations, malformed packets, unsupported versions and ungranted capabilities fail closed.

## Build, install and test

    make -C Sources/Darkin test
    make -C Sources/XCM test
    make test-darkin
    make test-xcm

The root filesystem build installs the Darkin service, `xcmd`, `libxcm.a`, public headers and enables both systemd units. XCM starts only after the Darkin backend endpoint is available.

## Deliberate implementation limits

This is an operational first broker milestone, not yet the complete production message bus. The following are still required before claiming full XCM completion:

- A registered Choca renderer endpoint and token delivery.
- More Darkin service endpoints and typed request dispatch beyond system information.
- Per-application identities/capabilities instead of the initial user-session group policy.
- Durable, bounded audit records with rotation and structured event IDs.
- Cryptographic update-bundle verification and rollback integration.
- A confirmation-grant issuer and atomic single-use grant store before enabling deletion.
- Stronger replay/session semantics and fuzzing of the wire parser.
- Kernel-enforced isolation and a Choca application sandbox.

The current broker intentionally does not issue destructive capabilities or claim to perform operations that have no registered backend.

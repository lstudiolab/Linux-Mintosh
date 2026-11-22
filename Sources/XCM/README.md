# XUM — system messaging broker (legacy source location)

XPU is Lenix's standalone system communication and security service outside Sean and Cacao. It is the controlled bridge between Cacao clients and Sean-owned backend services. This directory remains named `Sources/XCM/` temporarily while source files, public headers and protocol symbols are migrated.

The broker is built and installed as `xpu`. It listens on `/run/mintosh/xpu.sock`. The unprivileged client library and wire structs still use the legacy `xcm_*` API names.

## Implemented components

- `xcmd.c`: XPU broker executable source, built as `xpu`; Linux local IPC over Unix `SOCK_SEQPACKET` and kernel `SO_PEERCRED`.
- `policy.c`: deny-by-default C17 authorization policy.
- `usi_token_validation.c`: strict validator for Cacao's presentation-only token stream.
- `client.c`: Cacao-side request/reply client library.
- `sean_client.c`: private XPU-to-Sean backend client (legacy filename/API).
- `include/xcm/protocol.h`: versioned request/response wire envelopes and typed payload identifiers (legacy include path).
- `include/xcm/usi_tokens.h`: bounded USI presentation token contract.
- `Sources/Services/xpu.service`: confined systemd service.
- Host tests for policy decisions, client transport framing and USI token validation.

## Request path

1. Cacao connects to `/run/mintosh/xpu.sock` using a local sequenced-packet socket.
2. XPU obtains peer PID, UID and primary GID from the kernel, then verifies the process's actual supplementary groups through `/proc/<pid>/status` include `mintosh-services`. Root is not accepted as a Cacao client.
3. XPU validates packet size, protocol version, request ID, flags, payload type, target, operation, object scope and capabilities.
4. A bounded recent-request cache keyed by kernel-authenticated UID, PID and request ID rejects duplicate requests.
5. The initial broker grants only presentation and read-only system-information capabilities. User-data write/delete and foundation-update capabilities are not issued.
6. Presentation payloads pass a strict validator for bounded token count/depth, known presentation-only token types, UTF-8, balanced containers, numeric resource IDs and action IDs. There are no shell, executable, filesystem-path or arbitrary-command tokens.
7. An authorized system-information request is forwarded over the root-only `/run/sean/system.sock` endpoint. Sean authenticates the XPU process and independently validates the internal request.
8. Valid presentation requests are policy-checked but return `UNAVAILABLE` until the Cacao renderer endpoint is registered.
9. Policy denials and accepted system-information queries are written to the system authentication log.

## Security properties

- Cacao cannot mutate Sean, XCU, boot assets, Cacao core, XPU core or security policy.
- Arbitrary executable payloads are always rejected.
- USI tokens are presentation data, never privileged commands.
- Local peer identity is kernel-derived, never supplied by the request.
- The Cacao-facing broker socket is root-owned, group-restricted and mode `0660`.
- The Sean backend socket is root-owned, mode `0600`, and placed in a root-only runtime directory. Cacao cannot connect to it directly.
- Both services run with reduced capability sets and systemd sandboxes.
- Unknown operations, malformed packets, unsupported versions and ungranted capabilities fail closed.

## Build, install and test

    make -C Sources/XPU test
    make test-xpu

The root filesystem installs the `xpu` executable, `libxcm.a` and compatibility headers, then enables `xpu.service` after `sean-system.service`.

## Deliberate implementation limits

This is an operational first broker milestone, not yet the complete production message bus. Remaining work includes:
- A registered Cacao renderer endpoint and token delivery.
- More Sean service endpoints and typed dispatch beyond system information.
- Per-application identities/capabilities instead of the initial user-session group policy.
- Durable, bounded audit records with rotation and structured event IDs.
- Cryptographic update-bundle verification and rollback integration.
- A confirmation-grant issuer and atomic single-use grant store before enabling deletion.
- Stronger replay/session semantics and parser fuzzing.
- Kernel-enforced isolation and a Cacao application sandbox.

The current broker intentionally does not issue destructive capabilities or claim to perform operations that have no registered backend.

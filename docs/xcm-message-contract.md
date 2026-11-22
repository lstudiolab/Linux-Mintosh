# XCM message contract

XCM is a standalone message broker between Choca and Darkin, not a shared-memory backdoor. The first transport uses Linux Unix-domain `SOCK_SEQPACKET` sockets so each request arrives as one bounded packet.

## Message directions and payloads

| Direction | Message class | Meaning |
|---|---|---|
| Choca → XCM → Choca renderer | UI token stream | USI presentation output: layout, text, controls and visual state |
| Choca → XCM → Darkin service | Typed request | Declared operation against a capability-scoped object |
| Darkin service → XCM → Choca | Darkin data | Structured system state or query result |
| Darkin service → XCM → Choca | Darkin event | Hardware, storage, network, power or service state change |
| Darkin service → XCM → Choca | Service status | Success, denial, unavailable or bounded error result |
| Darkin service → XCM → Choca | App event | Application lifecycle and package-operation status |

USI tokens are a restricted presentation language. They are not shell commands, executable code, filesystem paths or kernel requests. The v1 token contract supports bounded UTF-8 text, balanced row/column containers, spacers, buttons with numeric action IDs and images with numeric resource IDs.

## Request invariants

- Protocol version must be supported.
- Request ID must be nonzero and echoed in the response.
- Message and payload sizes are bounded before parsing.
- Peer PID/UID/GID are obtained through kernel socket credentials, not request fields.
- The initial broker accepts only non-root users whose actual process groups include `mintosh-services`.
- Every operation has an allowlisted target and payload type.
- Presentation streams pass strict token count/depth, UTF-8, payload-size and token-type validation before they can be routed.
- The first broker grants only presentation and read-only system-information capabilities.
- User-data operations and foundation updates remain disabled until their backend, grant lifecycle and update-verification services exist.
- Darkin kernel, boot assets, security policy, Choca core and XCM core have no Choca mutation operation.
- Darkin services independently authorize each request.

## Current broker behavior

The C17 broker and client library are built by `Sources/XCM/Makefile`. The broker listens at `/run/mintosh/xcm.sock`, validates requests, maintains a bounded recent-request replay cache keyed by authenticated UID/PID and request ID, and audits policy decisions through the system authentication log.

For a system-information query, XCM forwards a separate typed request to the Darkin system service over `/run/mintosh-darkin/system.sock`. That backend socket is root-only and authenticates the XCM service credentials. Darkin independently validates the operation and returns a bounded typed response. Presentation streams are structurally validated but return `UNAVAILABLE` until a Choca renderer endpoint is registered.

The replay cache is bounded and is not a substitute for a durable session protocol. A production release still requires session-bound nonces, a stronger replay window, parser fuzzing, endpoint registration and per-application capabilities.

## Trust separation

Darkin is the backend foundation and does not depend on Choca. Choca cannot directly open privileged Darkin interfaces. XCM validates and routes typed messages but does not grant privileges that Darkin itself does not enforce.

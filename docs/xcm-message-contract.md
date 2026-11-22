# XPU message contract

XPU is a standalone message broker between Cacao and Tarts, not a shared-memory backdoor. The current transport uses Linux Unix-domain `SOCK_SEQPACKET` sockets so each request arrives as one bounded packet. The wire structs still use legacy `xcm_*` names while the source/API migration proceeds.

## Message directions and payloads

| Direction | Message class | Meaning |
|---|---|---|
| Cacao → XPU → Cacao renderer | UI token stream | USI presentation output: layout, text, controls and visual state |
| Cacao → XPU → Tarts service | Typed request | Declared operation against a capability-scoped object |
| Tarts service → XPU → Cacao | Tarts data | Structured system state or query result |
| Tarts service → XPU → Cacao | Tarts event | Hardware, storage, network, power or service state change |
| Tarts service → XPU → Cacao | Service status | Success, denial, unavailable or bounded error result |
| Tarts service → XPU → Cacao | App event | Application lifecycle and package-operation status |

USI tokens are a restricted presentation language. They are not shell commands, executable code, filesystem paths or kernel requests. The v1 token contract supports bounded UTF-8 text, balanced row/column containers, spacers, buttons with numeric action IDs and images with numeric resource IDs.

## Request invariants

- Protocol version must be supported.
- Request ID must be nonzero and echoed in the response.
- Message and payload sizes are bounded before parsing.
- Peer PID/UID/GID are obtained through kernel socket credentials, not request fields.
- The initial broker accepts only non-root users whose actual process groups include `mintosh-services`.
- Every operation has an allowlisted target and payload type.
- Presentation streams pass strict token count/depth, UTF-8, payload-size and token-type validation before routing.
- The first broker grants only presentation and read-only system-information capabilities.
- User-data operations and foundation updates remain disabled until their backend, grant lifecycle and update-verification services exist.
- Tarts kernel, boot assets, security policy, Cacao core and XPU core have no Cacao mutation operation.
- Tarts services independently authorize each request.

## Current broker behavior

The C17 broker is built and installed as `xpu`, listens at `/run/mintosh/xpu.sock`, validates requests, maintains a bounded recent-request replay cache keyed by authenticated UID/PID and request ID, and audits policy decisions through the system authentication log. Its implementation and compatibility API remain under `Sources/XCM/`.

For a system-information query, XPU forwards a separate typed request to the Tarts system service over `/run/tarts/system.sock`. That backend socket is root-only and authenticates the XPU service credentials. Tarts independently validates the operation and returns a bounded typed response. Presentation streams are structurally validated but return `UNAVAILABLE` until a Cacao renderer endpoint is registered.

The replay cache is bounded and is not a substitute for a durable session protocol. A production release still requires session-bound nonces, a stronger replay window, parser fuzzing, endpoint registration and per-application capabilities.

## Trust separation

Tarts is the backend foundation and does not depend on Cacao. Cacao cannot directly open privileged Tarts interfaces. XPU validates and routes typed messages but does not grant privileges that Tarts itself does not enforce.

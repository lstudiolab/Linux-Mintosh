# XCM message contract

XCM is a message broker, not a shared-memory backdoor. Each component communicates through versioned envelopes and an authenticated local transport. Messages are routed to named service endpoints; there is no generic "run this command as root" endpoint.

## Message directions and payloads

| Direction | Message class | Meaning |
|---|---|---|
| Choca → XCM → Choca renderer | UI token stream | USI presentation output: layout, text, controls and visual state |
| Choca → XCM → Darkin service | Typed request | A declared operation against a capability-scoped object |
| Darkin service → XCM → Choca | Darkin data | Structured system state or query result |
| Darkin service → XCM → Choca | Darkin event | Hardware, storage, network, power or service state change |
| Darkin service → XCM → Choca | Service status | Success, denial, unavailable or bounded error result |
| Darkin service → XCM → Choca | App event | Application lifecycle and package-operation status |

UI tokens are a presentation language only. They are not shell commands, executable code, filesystem paths or kernel requests. XCM routes them only to a Choca presentation endpoint.

## Request invariants

- Protocol version must be supported.
- Request ID must be nonzero. The broker must track IDs per authenticated session and reject replays; replay tracking is a transport/session responsibility and is not yet implemented in the policy-only prototype.
- Message and payload sizes are bounded before parsing.
- Peer identity and capability grants come from authenticated broker state, never from request fields.
- Every operation has an allowlisted target and payload type.
- User-data operations require object-scoped handles and capabilities.
- Delete requires a one-time confirmation grant bound to the exact operation and object; XCM consumes that grant atomically before dispatch.
- Darkin kernel, boot assets, security policy and system binaries have no Choca mutation operation.
- Foundation updates are accepted only from the isolated update service after bundle signature verification and rollback validation.
- Darkin services independently authorize each request.

## Trust separation

Darkin is the backend foundation and does not depend on Choca. Choca cannot directly open privileged Darkin interfaces. XCM validates and routes typed messages but does not grant privileges that Darkin itself does not enforce.

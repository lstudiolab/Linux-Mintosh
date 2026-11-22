# XCM — system communication and security broker

XCM is a standalone system component outside Darkin and Choca. It is the only supported communication bridge between Choca and Darkin services.

## Request path

1. Choca sends a versioned typed message to XCM over a local authenticated IPC transport.
2. XCM validates peer identity, protocol version, message length, operation, target class and capability.
3. XCM rejects malformed, unknown, out-of-scope or protected-foundation mutations.
4. XCM forwards an approved request to the specific Darkin service, not to a generic privileged shell.
5. Darkin independently checks authorization and performs the operation.
6. XCM returns a typed response/event to Choca.

## Trust boundary

Choca UI tokens are presentation data only. They are never interpreted as privileged Darkin commands. Choca cannot delete or rewrite Darkin kernel images, boot assets, system binaries, service implementations or security policy. Darkin updates require a separate signed update and rollback flow.

User files are different from foundation code: user-data operations may be exposed through dedicated APIs with narrow capabilities, object-scoped targets and explicit confirmation for deletion.

The initial policy library is deliberately deny-by-default. Transport authentication, capability issuance/storage, audit persistence and service-side enforcement remain required before production use.

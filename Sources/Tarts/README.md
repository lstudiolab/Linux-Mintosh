# Tarts

Tarts is the canonical name for Mintosh's complete backend foundation. It owns the Linux-based XCU integration, boot/runtime, hardware and device support, system services, security, storage, networking, power, logging and service lifecycle.

The first working backend implementation is retained in `../Darkin/` while it is migrated. This directory provides the canonical build entry point without duplicating or prematurely moving source files.

Run `make -C Sources/Tarts test` to build and execute the current backend service smoke test. The current service is limited to read-only system information; Tarts is not yet a complete backend.

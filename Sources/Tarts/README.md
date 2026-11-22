# Tarts

Tarts is the canonical name for Mintosh's complete backend foundation. It owns the Linux-based XCU integration, boot/runtime, hardware and device support, system services, security, storage, networking, power, logging and service lifecycle.

The Tarts system-information service is built and installed as `tarts-system`; it listens privately on `/run/tarts/system.sock` and accepts requests only from the authenticated XPU service. Its source remains in `../Darkin/` while the directory/API migration is completed. This directory provides the canonical build entry point without duplicating source files.

The native `launch7762` supervisor is the initial Tarts service-lifecycle implementation. It now supports deterministic `Order` values, per-service primary `Group` isolation, bounded restart policies and trusted executable validation. Its current source remains under `Sources/Launch7762/`.

Run `make -C Sources/Tarts test` to build and execute the current backend service smoke test. The current Tarts system endpoint is limited to read-only system information; the complete backend and service catalogue are not yet implemented.

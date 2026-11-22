# Tarts

Tarts is Mintosh's backend foundation above the Linux/XCU kernel. It is responsible for the non-graphical operating system runtime and must work without a graphical session.

## Ownership

- Native init and PID 1 startup.
- System service manager, dependency graph, launch, supervision and recovery.
- Hardware/device coordination and system daemons.
- Storage, filesystems, networking and connectivity.
- Security policy, credentials integration and privileged operations.
- Power, thermal, logging, diagnostics and update/recovery coordination.
- Core Services runtime and shared service contracts.

Core Services is a distinct API/service layer within Tarts. Tarts owns the backend and service lifecycle; Core Services exposes stable, least-privilege interfaces to Cacao and other clients.

Tarts must not own Window Server, Cacao, desktop UI or the application framework. Those components use Core Services and XPU.

## Current implementation

The system-information service is built as `tarts-system` and listens on `/run/tarts/system.sock`. The native service supervisor is exposed as `tarts-services` (legacy source: `Sources/Launch7762/`). Native PID 1 exists as `mintosh-init`, but it is not yet the default init in the transitional ISO/rootfs; that image still uses Debian/systemd.

The current endpoint only provides read-only system information. The full Tarts service catalogue, native boot integration, console/login, recovery and installer lifecycle are not complete.

Run `make -C Sources/Tarts test` for the current backend smoke test.

# Sean — system foundation

Sean is Lenix's non-graphical system foundation above XCU. It owns the trusted backend runtime and the services required to make the machine usable before any graphical session exists.

Sean owns:
- Native init and system service lifecycle.
- Hardware and device service coordination.
- Storage, filesystems and networking.
- Security enforcement, credentials and policy.
- Logging, diagnostics, power and recovery.
- Core Services daemons and their service contracts.

Core Services is a distinct public API/service layer within Sean. Sean owns the backend runtime and supervises its services; Core Services defines stable APIs used by Choca and other clients.

Sean is independent of Choca and Window Server. Graphical applications reach protected Sean services through XUM and Core Services contracts.

The canonical implementation and public protocol live in this directory. New backend code belongs under `Sources/Sean/`.

# Tarts — legacy source root

This directory is a temporary migration location. **Tarts** is the canonical name and `Sources/Tarts/` is the canonical component root.

Tarts owns the non-graphical backend foundation:
- Linux/XCU integration and early userspace.
- Native init and system service lifecycle.
- Hardware, device, memory and process services.
- Storage, filesystems and networking.
- Security enforcement, credentials and policy.
- Logging, diagnostics, power and recovery.
- Core Services daemons and their service contracts.

Core Services is a distinct public API/service layer *inside the Tarts foundation*. Tarts owns the backend runtime and supervises its services; Core Services defines stable APIs used by Cacao and other clients.

Tarts is independent of Cacao and Window Server. Graphical applications reach protected Tarts services through XPU and Core Services contracts.

The existing implementation remains in this legacy directory until its source and build integration are migrated. New implementation must be added under `Sources/Tarts/`, not here.

# Tarts — backend foundation (legacy source location)

The backend foundation's final name is **Tarts**. This directory remains named `Sources/Darkin/` temporarily so existing references and build integration are not broken during migration.

Tarts owns the complete non-graphical foundation:
- Linux kernel integration through XCU.
- Boot and runtime infrastructure.
- Hardware, device, memory and process services.
- Storage, filesystems and networking.
- Security enforcement, logging and power management.
- System service lifecycle, launch, dependency ordering, supervision and recovery.

Shared, stable user-space APIs and reusable system functionality are organized as **Core Services**. Tarts owns and supervises their service processes, but Core Services is a distinct API/service layer.

Tarts is independent of Cacao. It does not contain the desktop, graphical UI, Window Server, graphical framework or app-manager interface. Cacao reaches Tarts-owned services through the XPU security and IPC boundary.

The existing Linux-backed XNV primitives and read-only system-information service are prototypes being migrated to XCU and Tarts contracts. Do not rename implementation paths until build and CI integration has been updated and tested.
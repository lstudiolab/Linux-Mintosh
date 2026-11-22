# Mintosh native source tree

Mintosh's native foundation is implemented in C, C++ and architecture-specific assembly. The target component ownership is defined in [docs/platform-architecture.md](../docs/platform-architecture.md).

## Target ownership and current paths

- **XCU Kernel** — upstream Linux plus Mintosh's kernel integration and BSD-derived/Mach-inspired interfaces. Current paths: `kernel/linux/`, `kernel/xnv/`, and Linux-backed primitives in `Sources/Darkin/XNV/`.
- **XPU** — authenticated IPC and security broker. Current prototype: `Sources/XCM/`.
- **Tarts** — complete backend foundation, including kernel integration, boot/runtime, hardware, security, system services, service launch and supervision. Current prototype root: `Sources/Darkin/`.
- **Core Services** — stable system APIs and shared user-space services. Existing candidates include `Sources/Foundation`, `Sources/LibSys`, `Sources/GVD`, `Sources/IOPS`, `Sources/IOPlus`, `Sources/DM`, `Sources/FMS`, `Sources/Security` and `Sources/SystemLogging`. These are prototypes, not all completed services.
- **Window Server** — planned display, window, surface, composition and input service. It is not yet implemented.
- **Cacao** — graphical platform and native application framework. It will own the desktop-facing UI, application manager, application lifecycle APIs, controls, layout and public SDK. Current legacy design path: `Sources/Choca/`.

`Sources/XNV/` is a retained standalone EFI bootstrap experiment and is not the selected kernel path. The actual kernel is upstream Linux plus `kernel/xnv/` integration and Linux-backed primitives.

Do not rename implementation directories until the corresponding migration is complete, tested and reflected in build/CI integration.

## Build and test

    make verify
    make kernel-fetch
    make kernel-config
    make kernel
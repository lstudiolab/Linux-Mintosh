# Mintosh native source tree

Mintosh's native foundation is implemented in C, C++ and architecture-specific assembly. The target component ownership is defined in [docs/platform-architecture.md](../docs/platform-architecture.md).

## Canonical components

- **XCU Kernel** — upstream Linux plus Mintosh's kernel integration and BSD-derived/Mach-inspired interfaces. Canonical build entry point: `Sources/XCU/`; public API/library: `<xcu/xcu.h>` and `libxcu.a`; current implementation: `kernel/linux/`, `kernel/xnv/` and `Sources/Darkin/XNV/`.
- **XPU** — authenticated IPC and security broker. Canonical build entry point: `Sources/XPU/`; public API/library: `<xpu/xpu.h>` and `libxpu.a`; current implementation: `Sources/XCM/`.
- **Tarts** — complete backend foundation, including kernel integration, boot/runtime, hardware, security, system services, service launch and supervision. Canonical build entry point: `Sources/Tarts/`; current backend implementation: `Sources/Darkin/`.
- **Core Services** — stable system APIs and shared user-space services. Canonical build entry point: `Sources/CoreServices/`; initial public API/library: `<coreservices/coreservices.h>` and `libcoreservices.a`; implementation libraries and service APIs are under `Sources/CSF/`, `Sources/Foundation/`, `Sources/LibSys/` and selected service modules. The aggregate native build remains in `Sources/MintoshCore/`.
- **Window Server** — planned display, window, surface, composition and input service. It is not yet implemented.
- **Cacao** — graphical platform and native application framework. It will own the desktop-facing UI, application manager, application lifecycle APIs, controls, layout and public SDK. Current legacy design path: `Sources/Choca/`.

## Migration policy

Canonical Makefiles currently forward to the first tested implementation paths. This keeps build and root-filesystem packaging on the final component names without duplicating code or prematurely moving source files.

`Sources/XNV/` is a retained standalone EFI bootstrap experiment and is not the selected kernel path. The actual kernel is upstream Linux plus `kernel/xnv/` integration and Linux-backed primitives.

Do not remove legacy implementation paths until the source migration, public API compatibility, packaging and CI tests have all passed.

## Build and test

    make verify
    make test-tarts
    make test-xpu
    make test-xcu
    make test-core-services
    make kernel-fetch
    make kernel-config
    make kernel

# Lenix

Lenix is a Linux-based operating system with a layered architecture separating the kernel, system services, graphical infrastructure and application frameworks. Lenix uses its own implementations and APIs and does not depend on Apple source code.

## Platform architecture

The canonical target architecture is documented in [docs/platform-architecture.md](docs/platform-architecture.md).

- **XCU Kernel** — Lenix's Linux-based hybrid-kernel architecture. Linux is the actual primary kernel; BSD-derived interfaces provide Unix/POSIX compatibility; Mach-inspired task, port and messaging abstractions are implemented using Linux primitives.
- **XUM / XUM migration** — the existing authenticated IPC broker and the planned system-wide messaging interface. These are user-space communication facilities, not another kernel.
- **Sean / Sean migration** — the existing backend foundation and service lifecycle implementation.
- **Core Services** — stable shared user-space APIs and services for storage, networking, sessions, notifications, security clients, logging and application lifecycle.
- **Window Server** — display management, windows, surfaces, composition, focus and input routing.
- **Choca** — the graphical platform and native application framework, including desktop UI, application manager, application lifecycle, controls, layout, accessibility and public application APIs.

The source tree retains historical directory names while components are migrated and tested. See [docs/upstream-integration.md](docs/upstream-integration.md) for the upstream code integrated into the Lenix architecture and the license/provenance record.

## Linux kernel model

Linux is the real kernel and provides scheduling, virtual memory, drivers, VFS, networking, hardware support and kernel security. BSD-derived functionality is a first-party compatibility layer, not a second FreeBSD kernel. Mach-inspired messaging and task/port abstractions are implemented over Linux primitives. Lenix does not combine independently booted kernels.

## Current implementation

The current preview boots upstream Linux with a transitional Debian userspace. XCU has initial Linux-backed primitives and configuration; the backend, graphical server, Choca desktop and native application SDK remain in development. Do not describe the current preview as a finished desktop or production-secure OS.

The selected kernel path is upstream Linux plus the XCU integration configuration under `kernel/xcu/`. The public XCU primitives are installed as `libxcu.a` with `<xcu/xcu.h>`.

## Build and verification

    make verify
    make kernel-fetch
    make kernel-config
    make kernel
    make iso
    make test-qemu

The OS foundation uses C, C++ and architecture-specific assembly. Build configuration and CI scripts are tooling, not OS implementation languages.

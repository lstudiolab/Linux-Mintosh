# Mintosh

Mintosh is a Linux-based operating system with a layered architecture inspired by the separation of kernel, system services, graphical infrastructure and application frameworks in macOS. Mintosh uses its own implementations and APIs; it does not copy Apple's proprietary code.

## Platform architecture

The canonical target architecture is documented in [docs/platform-architecture.md](docs/platform-architecture.md).

- **XCU Kernel** — Mintosh's Linux-based hybrid-kernel architecture. Linux is the actual primary kernel; BSD-derived interfaces provide Unix compatibility; Mach-inspired task, port and messaging abstractions are implemented using Linux primitives.
- **XPU** — authenticated inter-process communication, typed message routing, capability checks and security policy between components.
- **Tarts** — the complete backend foundation, including service lifecycle, launch, supervision, boot/runtime services, hardware and device services, storage, networking, security and system services.
- **Core Services** — stable shared user-space APIs and services for storage, networking, sessions, notifications, security clients, logging and application lifecycle.
- **Window Server** — display management, windows, surfaces, composition, focus and input routing.
- **Cacao** — the graphical platform and native application framework, including desktop UI, application manager, application lifecycle, controls, layout, accessibility and public application APIs.

Conceptually, applications use Cacao; Cacao uses Window Server and Core Services; Tarts owns backend services; cross-component requests use XPU; XCU provides the kernel primitives beneath them. Not every operation is required to pass through one serial call chain.

## Linux kernel model

Linux is the real kernel and provides scheduling, virtual memory, drivers, VFS, networking, hardware support and kernel security. BSD-derived functionality is a first-party compatibility layer, not a second FreeBSD kernel. Mach-inspired messaging and task/port abstractions are implemented over Linux primitives. Mintosh does not combine three independently booted kernels and does not copy XNU source.

## Current implementation and migration

The repository contains an earlier implementation milestone under legacy source names:
- `Sources/Darkin/` contains the initial backend foundation implementation and is being migrated to **Tarts**.
- `Sources/Darkin/XNV/` and `kernel/xcu/` contain the Linux-backed kernel abstractions that are migrating to **XCU Kernel**.
- `Sources/XCM/` contains the initial authenticated broker that is migrating to **XPU**.
- `Sources/Choca/` contains the earlier graphical-platform design and is being migrated to **Cacao**; its window-management responsibilities belong to the separate **Window Server**.
- Existing backend prototypes are being divided between **Tarts** and **Core Services**.

These implementation paths are retained until their replacements are tested. The current preview boots upstream Linux with a transitional Debian userspace. XCU/XPU have only their initial prototype capabilities; Tarts, Core Services, Window Server, Cacao, the desktop and native application SDK are not yet complete. Do not describe the current preview as a finished desktop or production-secure OS.

The earlier standalone EFI bootstrap under `Sources/XNV` is a retained experiment, not the selected kernel path. The actual kernel path is upstream Linux plus the canonical XCU integration configuration under `kernel/xcu/`. The public XCU primitives are installed as `libxcu.a` with `<xcu/xcu.h>`; XNV remains a compatibility ABI.

## Build and verification

    make verify
    make kernel-fetch
    make kernel-config
    make kernel
    make iso
    make test-qemu

The OS foundation uses C, C++ and architecture-specific assembly. Build configuration and CI scripts are tooling, not OS implementation languages.
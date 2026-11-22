# Mintosh architecture

The canonical target architecture is documented in [platform-architecture.md](platform-architecture.md). This file records the core design principles and migration constraints.

## XCU Kernel

XCU is the Mintosh kernel architecture and integration layer. Linux is the actual primary kernel and supplies scheduling, virtual memory, VFS, networking, drivers, device model, interrupts, hardware support and kernel security.

BSD-derived interfaces provide first-party Unix/POSIX and selected BSD compatibility on Linux facilities. They are not a second BSD kernel.

Mach-inspired functionality provides task/thread abstractions, IPC messages, port-like endpoints, message rights and memory-object concepts backed by Linux primitives. These do not replace Linux's scheduler or memory manager. XCU is not three independently booted kernels.

The current implementation is under `Sources/Darkin/XNV/` and `kernel/xnv/` pending a tested migration to XCU naming. The standalone EFI experiment in `Sources/XNV/` is not the selected kernel.

## XPU

XPU is the authenticated communication and security service. It evolves the current XCM broker under `Sources/XCM/`. XPU validates typed, versioned messages, authenticates local peers, applies capability policy, routes requests and responses, and audits denials.

Kernel peer credentials and kernel isolation remain authoritative. Every receiving service must independently authorize requests. XPU is not a replacement for kernel security and is not a universal mandatory hop for every internal function call.

## Tarts — backend foundation

Tarts is the complete backend foundation. It owns boot/runtime infrastructure, kernel integration, hardware and device services, process and memory services, storage, filesystems, networking, security enforcement, logging, power management and system services.

Tarts also owns service lifecycle responsibilities: dependency ordering, launch, supervision, restart, health and service registration. Existing Darkin backend prototypes are being migrated into Tarts or split into Core Services as appropriate.

## Core Services

Core Services owns stable user-space system APIs and shared services such as storage, networking, user sessions, notifications, preferences, security clients, logging and application lifecycle support.

Tarts supervises service processes; Core Services provides shared APIs and service functionality. These are distinct responsibilities.

## Window Server

Window Server owns display topology, windows, surfaces, composition, focus and input routing. It is a system service, not an application framework. It treats client surfaces and input as untrusted.

## Cacao — graphical platform and application framework

Cacao is Mintosh's graphical platform and native application framework. It owns the desktop-facing experience, application manager and native application APIs. It provides application lifecycle, event loops, controls, views, layout, accessibility, drawing, dialogs and stable SDK APIs.

Cacao clients use Window Server's documented client protocol; applications do not receive privileged access to display devices.

## Trust and privilege boundaries

- Kernel enforcement governs process identity, memory isolation, device access and mandatory controls.
- XPU authenticates and authorizes cross-component IPC.
- Each service validates and authorizes its own operations.
- Tarts supervises services but does not grant privileges.
- Cacao applications run with per-application identities and least privilege.
- Window Server treats client surfaces and input as untrusted.
- Signed system updates require verification, atomic activation and rollback. General IPC cannot rewrite the kernel, boot chain or security policy.

## Current prototype status

The current preview boots upstream Linux with a transitional Debian userspace. It has Linux-backed XNV task/port/memory/event primitives, an initial XCM broker and a limited read-only system-information endpoint. These are prototype implementations for the XCU/XPU and Tarts/Core Services migration.

Full XPU endpoint registration and per-app identities, Tarts orchestration, Core Services APIs, Window Server, Cacao, desktop shell and native application SDK remain implementation work. See [platform-architecture.md](platform-architecture.md) for the complete target and component migration table.

## Implementation languages

The OS foundation uses C, C++ and architecture-specific assembly only. Build-system configuration and CI files are tooling, not OS implementation languages.
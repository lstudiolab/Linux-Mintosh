# Mintosh platform architecture

This document defines the target architecture for Mintosh. It adopts the mature separation of responsibilities used by macOS as a design reference, while retaining independent Mintosh implementations and APIs.

## Architecture at a glance

```text
Applications
    │
    ▼
Cacao — native application and graphical framework
    │
    ├──────────────► Window Server — windows, displays, composition, input
    │
    ▼
Core Services — stable user-space system APIs and shared services
    │
    ▼
Tarts — complete backend foundation, system services and service lifecycle
    │
    ▼
XPU — authenticated IPC, message routing and security policy
    │
    ▼
XCU Kernel — Linux main kernel + BSD-derived Unix interfaces
              + Mach-inspired messaging/task/port abstractions
    │
    ▼
Hardware
```

The diagram shows the conceptual layers, not a requirement that every operation travel through one serial call chain. Components use the narrowest documented API appropriate to their trust level. XPU is the controlled cross-component IPC and authorization boundary; kernel primitives remain kernel-enforced.

## XCU Kernel

XCU is Mintosh's hybrid-kernel architecture and integration layer.

- **Linux is the actual, primary kernel.** It provides scheduling, virtual memory, process execution, VFS, networking, device drivers, interrupt handling, hardware support and kernel security mechanisms.
- **BSD-derived functionality** provides first-party Unix/POSIX and selected BSD-compatible interfaces on top of Linux. This is not a second BSD kernel.
- **Mach-inspired functionality** provides task/port/message abstractions, message rights and memory-object concepts using Linux primitives. It does not replace Linux's scheduler or memory manager.

XCU is not a collection of three independently booted kernels. It is one Linux-based kernel architecture with compatible interfaces and additional first-party abstractions.

## XPU

XPU is Mintosh's system communication and security service. It is inspired by the combination of Mach IPC concepts and Apple's XPC-style service boundaries.

Responsibilities:
- Authenticated local IPC and component identity.
- Typed, versioned messages and bounded payload validation.
- Endpoint registration and service discovery.
- Capability-based authorization and operation-specific policy.
- Message routing, replies, events and error reporting.
- Replay resistance, auditing and rate/resource limits.
- Explicit protection for kernel, boot, security policy and system components.

XPU must not trust caller-supplied identity, raw pointers, arbitrary paths, executable payloads or UI tokens as privileged instructions. The kernel authenticates local peers; XPU validates protocol and policy; each receiving service independently enforces authorization. XPU is not a substitute for kernel isolation.

The XPU broker is built and installed as `xpu` and listens on `/run/mintosh/xpu.sock`. Its implementation source remains under `Sources/XCM/` until a tested source/API migration is completed. The privileged Tarts system-information endpoint is `/run/tarts/system.sock`.

## Tarts — backend foundation

Tarts is the complete non-graphical backend foundation of Mintosh. It owns and contains the XCU Kernel integration, boot/runtime infrastructure, hardware and device services, process and memory services, storage, filesystems, networking, security enforcement, logging, power management and system services.

Tarts must operate without Cacao or the Window Server. It does not contain graphical UI, the desktop, application manager or native application framework.

Tarts also owns system service lifecycle and coordination:
- Start, stop, restart and supervise system services.
- Resolve service dependencies and startup ordering.
- Manage service health, crash recovery and resource limits.
- Coordinate system-level and user-session service instances.
- Provide controlled service registration and discovery through XPU.
- Record service lifecycle events.

Tarts contains the backend implementation; **Core Services** defines stable shared user-space APIs and service contracts used by applications and platform components. These responsibilities are related but should remain separate modules within the backend foundation.

The existing Tarts backend and launch/service-supervision prototypes are the initial Tarts implementation candidates. They must be audited and migrated to stable contracts rather than renamed blindly.

## Core Services

Core Services is the stable user-space system API and shared-service layer used by applications and platform components.

It will contain narrowly scoped APIs and daemons for areas such as:
- Files, volumes and storage coordination.
- Networking and connectivity.
- User accounts, sessions and preferences.
- Notifications, time, localization and accessibility.
- Security identities, credentials and authorization clients.
- Application lifecycle support and system events.
- Logging, diagnostics and system information.

Core Services APIs are not all privileged. Each service has its own identity, permissions and public contract. Privileged operations are implemented by a dedicated service and authorized independently; applications do not receive broad system access merely by linking a framework.

## Window Server

The Window Server owns the graphical session's windowing and display responsibilities:
- Display discovery, modes and topology.
- Window creation, ownership, stacking, focus and lifecycle.
- Surface allocation and composition.
- Input routing, hit testing and pointer/keyboard focus.
- Per-session isolation and application visibility.
- Display changes, scaling and multi-display behavior.

The Window Server is a system service, not an application framework. It uses kernel display/input facilities through controlled device interfaces and exposes a documented client protocol to Cacao and trusted platform components.

## Cacao

Cacao is Mintosh's native application and graphical framework, conceptually comparable to Cocoa.

It will provide:
- Application lifecycle and event-loop APIs.
- Standard controls, views, layout and accessibility.
- Windows and scenes through the Window Server client API.
- Text, images, animation and drawing abstractions.
- Menus, dialogs, preferences and common interaction patterns.
- Resource, localization and document APIs.
- A stable public SDK for native Mintosh applications.

Cacao is a framework, not the Window Server and not the kernel. Applications use Cacao APIs rather than calling privileged system services directly.

## Security and trust boundaries

1. Kernel enforcement is authoritative for memory isolation, process identity, device access and mandatory kernel controls.
2. XPU authenticates IPC peers, validates messages and enforces capability policy for cross-component requests.
3. Every receiving service performs its own authorization and input validation.
4. Tarts owns and supervises backend services but does not grant them privileges they do not already possess.
5. Cacao applications run with per-application identities and least-privilege access.
6. The Window Server treats application surfaces and input as untrusted.
7. System updates require signed artifacts, verification, atomic activation and rollback. No general-purpose IPC endpoint may rewrite the kernel, boot chain or security policy.

## Migration from the current prototype

The repository currently contains a working first milestone under earlier names. Preserve source paths until each migration is tested:

| Current prototype | Target architecture | Migration rule |
|---|---|---|
| `Sources/Darkin/XNV/` (legacy path) and `kernel/xcu/` | XCU Kernel | Retain Linux-backed pidfd, port, memory and event primitives; update public naming/contracts after compatibility tests |
| `Sources/XCM/` | XPU | Evolve the broker into a typed, authenticated IPC/security service; preserve deny-by-default policy |
| Tarts backend service prototypes | Tarts and Core Services | Migrate the complete backend foundation to Tarts; organize stable shared APIs as Core Services |
| Cacao graphical platform plan | Window Server and Cacao | Separate the window/compositor service from the public application framework |
| USI token prototype | Cacao rendering/API contract | Keep presentation data non-executable; do not treat UI tokens as privileged commands |

The earlier names remain in implementation paths for now and must not be mistaken for the final public architecture.

## Implementation constraints and status

The OS foundation is implemented in C, C++ and architecture-specific assembly. Build configuration and CI scripts are tooling, not OS implementation languages.

This architecture is a target specification, not a claim that every layer is already implemented. The current preview has a Linux-backed XCU primitive library (`libxcu.a`), an XPU broker (`xpu`, `libxpu.a`) with a restricted read-only system-information path, a Tarts backend endpoint (`tarts-system`), a first Core Services API (`libcoreservices.a`), and the native launch7762 supervisor with deterministic service ordering and per-service primary-group isolation. The current ISO/rootfs still uses a transitional Debian userspace and systemd-based boot; Tarts is not yet the PID 1 init/service manager for the complete system. Full XPU endpoint registration and per-app identities, a complete Tarts backend/service catalogue, Core Services APIs, the Window Server, Cacao, the desktop and native application SDK remain implementation work.

# Lenix platform architecture

This is the canonical architecture for Lenix. It adopts the proven *separation of responsibilities* used by macOS as a design reference, while keeping Lenix's own implementation, APIs, and Linux kernel. XCU is the rebranded and redesigned continuation of XNV; it is not a second kernel alongside XNV.

## System architecture

Lenix is organized as a layered platform with independently protected services. The layers describe ownership and dependency direction, not a single call stack through which every operation must pass.

```text
┌──────────────────────────────────────────────────────────────┐
│                         Applications                         │
│                 Native .app bundles / CLI tools              │
├──────────────────────────────────────────────────────────────┤
│                           Choca                              │
│       ApplicationKit · UI framework · app lifecycle          │
│       controls · views · drawing · accessibility · SDK       │
├───────────────────────────────┬──────────────────────────────┤
│         Window Server         │       Session Services       │
│ displays · windows · surfaces │ input · clipboard · launch   │
│ composition · focus · routing │ notifications · preferences  │
├───────────────────────────────┴──────────────────────────────┤
│                       Core Services                          │
│   stable APIs · storage · users · sessions · networking      │
│   security clients · logging · events · application services │
├──────────────────────────────────────────────────────────────┤
│                           Sean                              │
│     backend foundation · service lifecycle · system daemons   │
│       device services · storage · security · power · logs    │
├──────────────────────────────────────────────────────────────┤
│                            XCU                               │
│      Linux kernel integration · BSD/POSIX · Mach concepts     │
│       process · VM · IPC primitives · drivers · hardware     │
├──────────────────────────────────────────────────────────────┤
│                          Hardware                            │
└──────────────────────────────────────────────────────────────┘

XUM is the authenticated IPC and capability boundary used horizontally
between applications, Choca, Window Server, Core Services and Sean.
It is not another kernel layer and not a mandatory hop for every call.
```

## 1. XCU — kernel architecture

**XCU is XNV's successor name and the single kernel architecture of Lenix.** All kernel integration, kernel-facing abstractions, and low-level process/memory/IPC primitives belong to XCU. There is no separate XNV subsystem.

XCU uses a Linux-first hybrid architecture:

- **Linux is the actual kernel.** It owns privileged execution, scheduling, virtual memory, process isolation, VFS, networking, device drivers, interrupts, hardware discovery and kernel security.
- **BSD-derived interfaces** provide the Unix/POSIX and selected BSD-compatible user-facing contracts on top of Linux. BSD is not a second booted kernel and does not replace Linux's VFS or networking implementation.
- **Mach-inspired abstractions** provide task/port/message and memory-object concepts using Linux mechanisms such as pidfds, sequenced-packet sockets, descriptor passing, sealed memory objects and event facilities. These abstractions do not introduce a second scheduler or VM manager.
- **Kernel modules and drivers** extend Linux through its supported module and driver interfaces. They are not macOS-style Kext binaries and must follow Linux's kernel ABI and licensing requirements.

XCU is therefore one integrated Linux-based kernel platform. It is not a custom Mach/BSD kernel, and the old standalone EFI kernel experiment is not part of the selected architecture. The normal boot path loads the Linux kernel; XCU configures, integrates and extends that kernel.

## 2. Sean — backend foundation

Sean is the non-graphical system foundation above XCU. It owns the trusted system runtime and the services required to make the machine usable before any graphical session exists.

Sean responsibilities:
- Early userspace, PID 1 and system startup.
- System service manager, dependency graph, ordering, supervision and recovery.
- Hardware and device service coordination.
- Storage and filesystem management.
- Network configuration and connectivity services.
- System security policy, credentials integration and privileged operations.
- Power, thermal, battery and platform management.
- System logging, diagnostics, crash reporting and update coordination.
- Runtime directories, service identities, resource limits and service health.

Sean must boot and operate without Choca, Window Server or a logged-in user. It must not contain desktop UI, application controls or graphical application APIs.

Sean is the foundation that *contains and runs* the backend. Core Services is a separately versioned API/service layer within that foundation; it is not a second competing backend.

## 3. Core Services — shared system APIs

Core Services provides stable, documented user-space contracts for platform components and applications. It hides implementation details of Sean daemons and avoids exposing privileged interfaces directly to applications.

Planned service families:
- File and volume coordination.
- User accounts, authentication and session state.
- Preferences, localization, time and notifications.
- Networking and connectivity.
- Security identity, credential and authorization clients.
- System events, logging, diagnostics and system information.
- Application registration, metadata and lifecycle support.
- Updates, installation transactions and recovery coordination.

Each daemon has a distinct identity and least-privilege policy. Linking a library never grants privileged access. Core Services clients use direct library calls for local non-privileged work and XUM for protected cross-process operations.

## 4. XUM — system communication and security

XUM is Lenix's system-wide authenticated message and capability broker. It is inspired by Mach port concepts and service-oriented IPC such as XPC, but uses a Lenix-defined protocol.

XUM provides:
- Kernel-authenticated peer identity.
- Typed, versioned request, response and event messages.
- Service endpoint registration and discovery.
- Capability and operation-level authorization.
- Bounded payloads, descriptor validation and resource limits.
- Replay protection, rate limiting and security audit records.
- Session-aware routing between applications and system services.

XUM is a *horizontal communication facility*. It connects clients to the service they are authorized to use; it is not placed beneath Sean as though all system calls pass through it. Linux kernel permissions remain authoritative, and each receiving service independently validates and authorizes its requests. XUM cannot grant a process permissions it does not possess.

## 5. Window Server — graphical session service

Window Server is a separate system service. It owns:
- Display discovery, display modes and topology.
- Window creation, ownership, stacking, focus and lifecycle.
- Client surface allocation and composition.
- Input routing, hit testing and pointer/keyboard focus.
- Per-session isolation and display scaling.
- Window decorations and system-level display transitions where appropriate.

Window Server runs as a protected session service and accesses graphics/input devices through XCU/Linux driver interfaces. Applications submit untrusted surfaces and receive only their own input events. Window Server is not part of Choca and is not an application framework.

## 6. Choca — native application platform

Choca is Lenix's native application and graphical framework, conceptually comparable in *role* to Cocoa. It is built above Core Services and the Window Server client protocol.

Choca provides:
- Application lifecycle, event loops and application delegate contracts.
- ApplicationKit: windows, scenes, menus, dialogs and standard controls.
- View hierarchy, layout, drawing, text and animation APIs.
- Accessibility, localization, themes and user preferences.
- Document, resource and application bundle APIs.
- A stable SDK and ABI for native Lenix applications.

Choca applications do not open privileged display devices or call backend daemons through private sockets. They use Choca APIs, Core Services contracts and XUM-authorized service interfaces.

## 7. Application model

Native applications are packaged as structured `.app` bundles with a manifest, executable, resources, icons, declared capabilities and version metadata. The application manager registers bundles, validates metadata and signatures, resolves dependencies, assigns an application identity and launches the executable with a restricted environment.

The application manager is a Choca-facing system application backed by Core Services and Sean installation/launch services. The package format and installer are separate concerns; no third-party installer implementation is assumed to be part of the system foundation.

## 8. Boot and launch sequence

The intended native boot sequence is:

1. UEFI firmware ssean the Lenix boot manager.
2. The boot manager loads the signed Linux kernel, initramfs and boot configuration.
3. Linux initializes hardware, drivers, memory, processes and the root filesystem.
4. Linux ssean Lenix's native init as PID 1.
5. Sean initializes runtime state and ssean essential services in dependency order.
6. XUM ssean and registers protected service endpoints.
7. Core Services ssean the system APIs and background daemons.
8. The login/session manager authenticates a user and creates an isolated session.
9. Window Server ssean for that session and initializes displays and input.
10. Choca launches the desktop shell and user applications.

The current ISO still uses transitional Debian userspace and systemd. This sequence is the target, not a claim that the native boot conversion is complete. The transition must preserve a usable console, recovery path and installer before systemd is removed.

## 9. Trust boundaries

- Linux/XCU enforces process, memory, device and kernel security boundaries.
- Sean supervises services but does not implicitly grant privileges.
- XUM authenticates peers and enforces message-level capability policy.
- Every receiving service validates its own inputs and authorization.
- Core Services exposes least-privilege APIs rather than raw privileged handles.
- Window Server isolates sessions and treats application surfaces as untrusted.
- Choca applications run under distinct identities and declared capabilities.
- Updates use verified artifacts, atomic activation and rollback.

## 10. Source ownership and migration

The canonical source ownership is:

| Component | Canonical source root | Ownership |
|---|---|---|
| XCU | `Sources/XCU/`, `kernel/linux/`, `kernel/xcu/` | Linux integration and kernel-facing abstractions |
| Sean | `Sources/Sean/` | Backend runtime, init, service manager and system daemons |
| Core Services | `Sources/CoreServices/` | Public system APIs and shared service contracts |
| XUM | `Sources/XUM/` | Authenticated IPC, routing and capability policy |
| Window Server | `Sources/WindowServer/` | Display, window, surface and input service |
| Choca | `Sources/Choca/` | Application framework, desktop and application manager |

Legacy source roots such as `Sources/Sean/`, `Sources/XNV/`, `Sources/XUM/` and `Sources/Choca/` are migration debt, not additional architecture layers. XNV must be folded into XCU; it must not remain a parallel kernel project. XUM must be folded into XUM. Sean must be folded into Sean. Choca must be folded into Choca. Compatibility aliases may exist temporarily at public ABI boundaries, but new code must use canonical names.

## Implementation status

The architecture above is the target architecture. The repository now has a canonical Linux-backed XCU primitive implementation under `Sources/XCU/` with its own `libxcu.a`, `<xcu/xcu.h>` API and tests. The earlier XNV implementation paths are frozen migration references. The repository also has a restricted XUM broker, a read-only Sean system-information endpoint, an initial Core Services API and a native service-supervisor prototype. The current ISO still uses a transitional Debian userspace and systemd. The Window Server, Choca framework, application manager, native session stack and complete service catalogue are not implemented. The native Sean init/service lifecycle is not yet the default boot path.

The OS foundation is implemented in C, C++ and architecture-specific assembly. Build configuration and CI scripts are tooling, not OS implementation languages.

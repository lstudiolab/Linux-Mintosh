# Lenix architecture

The canonical system design is documented in [platform-architecture.md](platform-architecture.md). XCU is the renamed and redesigned continuation of XNV. There is one Linux-based kernel architecture, not parallel XCU and XNV kernels.

## Component ownership

- **XCU:** Linux kernel integration and low-level kernel-facing abstractions. Linux owns scheduling, memory management, process isolation, VFS, networking, drivers, interrupts and hardware support. BSD-derived interfaces and Mach-inspired task/port/message abstractions are integrated using Linux facilities.
- **Sean:** complete non-graphical backend foundation, including native init, service lifecycle, hardware/device services, storage, networking, security, power, logging and system daemons.
- **Core Services:** stable user-space APIs and shared service contracts within the Sean foundation.
- **XUM:** authenticated, typed IPC and capability broker. It is a horizontal communication boundary, not a mandatory serial layer beneath Sean.
- **Window Server:** protected graphical session service for displays, windows, surfaces, composition, focus and input routing.
- **Choca:** native application and graphical framework, application lifecycle, controls, views, accessibility, desktop shell and application manager.

## macOS-inspired design principles

Lenix adopts the *architecture principles* of macOS rather than Apple's proprietary implementation:
- A clear boundary between kernel, system services, graphical server and application frameworks.
- Small, independently supervised system services with narrow APIs.
- A separate Window Server and public application framework.
- Stable public frameworks above lower-level system APIs.
- Per-process identity, least privilege and explicit IPC capabilities.
- A consistent application bundle and lifecycle model.
- A native boot and service lifecycle independent of the graphical session.

Lenix remains Linux-based. It does not embed Apple's XNU, Darwin, Cocoa, XPC, launchd or private Apple source code.

## Boot dependency direction

UEFI → Lenix boot manager → Linux/XCU → Lenix init (PID 1) → Sean service manager → XUM and Core Services → user session → Window Server → Choca desktop → applications.

The current preview has not completed this transition: the ISO still boots transitional Debian userspace and systemd. Do not remove systemd until native PID 1, console login, recovery and installer paths are working.

## Security model

- Linux/XCU is authoritative for process, memory, device and kernel isolation.
- Sean supervises but does not grant privileges.
- XUM authenticates peers and enforces capability policy for cross-process messages.
- Each service performs its own authorization and input validation.
- Window Server isolates sessions and treats client surfaces as untrusted.
- Choca applications run with separate identities and least-privilege capabilities.
- Signed updates require verification, atomic activation and rollback.

## Current implementation status

The repository has early Linux-backed kernel primitives, an initial XUM broker, a limited Sean system-information service, a first Core Services API and a service-supervisor prototype. These are initial components, not a complete OS foundation. Window Server, Choca, native sessions, full service catalogue and native PID 1 boot integration remain outstanding.

The foundation implementation languages are C, C++ and architecture-specific assembly only. Build configuration and CI are tooling.

# Darkin — Mintosh system foundation

Darkin is the complete backend foundation. It owns XNV (the Linux-based kernel architecture), boot/runtime, hardware and device services, process and memory services, storage, filesystems, networking, security, logging, power and all non-graphical system services.

Darkin is independent of the graphical platform. It does not contain USI, a window server, graphical frameworks, desktop shell or app manager.

Existing backend prototypes in `Sources/` are Darkin-owned candidates. Their current Linux/POSIX dependencies and service boundaries must be audited before they are treated as final Darkin components.

Darkin exposes typed, versioned service APIs. It does not accept arbitrary code, pointers, paths or untrusted UI tokens as privileged commands. Choca reaches Darkin only through the separate XCM broker. Each Darkin service must enforce authorization itself even when XCM has already checked the request.\n\nThe first protected backend endpoint is `darkin-system`, which serves read-only system information only to the root XCM service over `/run/mintosh-darkin/system.sock`. The socket lives in a root-only runtime directory and is not accessible to Choca.

XNV is the Linux-based kernel architecture inside Darkin. The first-party XNV API in `Sources/Darkin/XNV` wraps Linux pidfds, Unix sequenced-packet ports with descriptor passing, sealed memfd memory objects and epoll event queues. These provide Mach-inspired task/port/memory abstractions without introducing a second kernel.

## Existing backend ownership map

These directories are Darkin-owned backend candidates. Their current implementations are prototypes and must be migrated to stable interfaces rather than assumed production-ready.

| Area | Existing source candidates | Responsibility |
|---|---|---|
| Boot and platform | `Sources/Boot`, `Sources/Architecture`, `Sources/Init`, `Sources/mBoot` | Boot handoff, platform startup and early system initialization |
| Core libraries | `Sources/Foundation`, `Sources/LibSys` | Base data structures, paths, files, descriptors and system API support |
| Process and services | `Sources/Launch7762`, `Sources/Launch511`, `Sources/LaunchClient`, `Sources/GVD`, `Sources/Darl` | Process lifecycle, service supervision, worker queues and service IPC |
| Hardware and power | `Sources/IOPS`, `Sources/IOPlus`, `Sources/HPCompatibility`, `Sources/DM`, `Sources/FMS`, `Sources/Kexts` | Hardware inventory, power, device and module management |
| Security and diagnostics | `Sources/Security`, `Sources/SystemLogging`, `Sources/CSF` | Permission checks, audit/logging and service-facing APIs |
| System update | To be implemented under Darkin | Signed OS image verification, atomic activation and rollback |

The `Sources/PackageManager` prototype must be split by responsibility: Choca owns the app-management experience, while Darkin owns privileged package verification, installation transactions and system integrity enforcement. The graphical installer belongs to Choca; privileged disk and boot changes must be exposed by narrowly scoped Darkin services through XCM.

## Non-goals

Darkin does not own the desktop, USI, window server, compositor, Finder-like shell, dock, graphical controls or app-manager UI. Those belong to Choca. Darkin does not communicate directly with Choca; XCM is the broker boundary.

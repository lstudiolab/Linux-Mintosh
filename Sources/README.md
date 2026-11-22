# Mintosh architecture

## Kernel direction: XNV

Mintosh's target kernel is **XNV**, a first-party hybrid kernel inspired by the broad Mach/BSD architecture of Apple's XNU. XNV is not Apple's XNU and does not reuse Apple's source. It will be implemented in first-party C and architecture-specific assembly, with C++ only where runtime and freestanding constraints are explicitly controlled.

- **Mach layer:** tasks, threads, scheduling, virtual memory, IPC, synchronization and port rights.
- **BSD layer:** POSIX process interfaces, VFS, file descriptors, sockets, networking, credentials and permissions.
- **XNV core:** boot handoff, CPU/platform abstraction, interrupt/exception dispatch, memory ownership, syscall entry and kernel diagnostics.
- **XCM:** first-party userspace system communication and session platform. It bridges system services and higher-level interfaces to kernel APIs; it is not part of the kernel.
- **USI:** future graphical interface built above XCM using the user's programming language. It is deferred until that language, compiler, runtime and standard library are complete.

Mach and BSD are integrated into one privileged hybrid kernel. XNV must be able to boot and operate without XCM or USI.

### Transition status

The current Developer Preview still boots the upstream Linux kernel and Debian-based userspace. That path is retained only as a temporary development/installer bootstrap while XNV is brought up. The final Mintosh architecture is not intended to depend on the Linux kernel or a third-party userspace. Do not remove the working preview path until XNV has a bootable image and passes automated boot, storage, device, networking, security and recovery tests.

The first XNV milestone is a stable bootloader handoff ABI. `Sources/XNV/include/xnv/boot.h` defines ABI v1 and `Sources/XNV/boot_contract.c` validates it. This is an early kernel foundation, not a bootable kernel.

## First-party system policy

All final Mintosh kernel, userspace, system services, frameworks, installer, recovery environment and package-management components are first-party implementations using C, C++ and assembly for the OS foundation. Standard hardware protocols and documented firmware interfaces are interfaces, not bundled third-party implementations. Build-host compilers, linkers, Make, GitHub Actions and QEMU are development/test tools and are not installed as substitutes for Mintosh components.

The current preview does not comply with the final architecture: it still uses the upstream Linux kernel, Debian Trixie, GRUB, systemd, udev, NetworkManager, GNU utilities, Debian package tooling and third-party firmware packages. This is explicitly transitional.

## Current preview boot sequence (legacy Linux path)

1. BIOS or UEFI firmware loads GRUB from the Mintosh USB image.
2. GRUB loads the release-pinned Linux kernel and compressed initramfs.
3. The initramfs mounts devtmpfs, procfs, sysfs and /run.
4. systemd starts as PID 1.
5. systemd starts device management, networking, the serial console, launch7762 and the Mintosh installer on tty1.
6. launch7762 starts enabled Mintosh-managed services, including Darl.

## Mintosh-owned native core

Native source code lives under `Sources/`. Most existing programs are Linux-userspace prototypes compiled inside the Debian target root filesystem. They are not yet an XNV userspace ABI and must be audited or ported to the final BSD/POSIX layer.

- **Mintosh Init (prototype):** first-party PID 1 process with pseudo-filesystem setup, launch7762 supervision, orphan reaping, bounded restart backoff and orderly shutdown. It is not yet selected by the installed boot path and does not yet provide a login/session manager.
- **Darl:** local Unix-socket system foundation API for status and version queries.
- **launch7762 / Launch Background:** native service supervisor with protected service definitions, direct `execve`, child lifecycle tracking, capped restart backoff and peer-credential authorization.
- **Launch Client:** per-user background service supervisor.
- **launch511:** standalone command execution helper.
- **LibSys:** POSIX utilities, bounded Unix-socket framing, descriptor helpers, runtime paths and logging.
- **CSF:** C client API for Darl.
- **Foundation:** C17 strings, byte data, arrays, dictionaries, URL parsing, error values, lexical paths and bounded atomic file I/O.
- **GVD:** process inspection/control and a bounded pthread task-dispatch library.
- **IOPS:** power and thermal inventory, CPU governor inspection/control and ACPI platform-profile management.
- **I/O+:** structured hardware inventory and Linux kernel uevent monitoring.
- **HP Compatibility:** C++ platform diagnostics backed by x86-64 assembly CPUID.
- **Kexts:** currently delegates Linux module operations to kmod; this must be replaced by XNV's native driver/module model.
- **Security:** baseline permissions audit.
- **System Logging:** current syslog/journald integration; final logging backend is an XNV-native service.

These are incremental userspace components. They do not constitute a completed kernel or foundation.

## Bootloader and kernel transition

The current mBoot is a native x86-64 UEFI application, but it loads the Linux EFI stub. It is not an XNV loader yet. XNV requires a defined executable image format, memory-map handoff, framebuffer/ACPI handoff, CPU-mode contract, page-table ownership and a kernel entry ABI. The initial boot contract is versioned in `Sources/XNV/include/xnv/boot.h`. The loader must be changed only alongside a bootable XNV image and QEMU tests.

The existing Linux kernel fetch/config/build targets remain available for the legacy preview. They are not the final kernel build pipeline.

## Implementation languages

The OS foundation uses C, C++ and architecture-specific assembly only. Kernel code must be freestanding: no host libc, no implicit runtime, explicit integer widths, bounded memory operations and documented ownership. Architecture-specific code belongs under an architecture directory. C++ must not introduce exceptions, RTTI, static constructors or standard-library dependencies into the early kernel unless those facilities are deliberately implemented and initialized by XNV.

The user's custom programming language is reserved for future application and graphical-system development. It does not replace C/C++/assembly for the kernel bring-up.

## Base userspace and packages

The current preview root filesystem is Debian Trixie-based and includes systemd, GNU core utilities, Bash, util-linux, NetworkManager, udev and external package tooling. This is a temporary development bootstrap and violates the final first-party system policy. It must be replaced with a Mintosh-built root filesystem after the XNV kernel and first-party userspace interfaces are ready.

`mstore` is the first-party package-manager interface, but native package search, metadata, database, installation, removal, dependency resolution, signature verification, repository updates and rollback are not implemented. Mintosh's native application format is `.pkg`, with a first-party manifest, payload, dependency model, permission policy, signature format, package database and transactional installer.

## Development phase policy

The active phase is kernel and system-foundation bring-up. XNV must boot to a native console, initialize memory and CPUs, provide process/thread and IPC primitives, expose a stable syscall and BSD/POSIX interface, mount storage, manage devices, establish networking, supervise services, enforce security boundaries, install/update packages safely, log failures and recover without a graphical session.

XCM is built after the kernel's IPC, process, device, display and input interfaces stabilize. USI begins only after the user's programming language, compiler, runtime and standard library are production-ready. No GUI implementation is part of the current XNV milestone.

## Graphical stack

USI is the future graphical interface. XCM supplies its system/session communication, display, input, window and application-lifecycle services. AppKit, Tarts, GT, Window Server, Finder and Dock remain deferred until XNV, XCM's required APIs and the user's programming language are ready.

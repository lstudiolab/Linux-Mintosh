# Mintosh architecture

## Target kernel: XNV

Mintosh is moving from the Linux-based Developer Preview to a first-party **XNV hybrid kernel**. XNV is inspired by the broad Mach/BSD architecture of Apple's XNU; it is an independent implementation and does not reuse Apple's source.

- **Mach subsystem:** tasks, threads, scheduling, virtual memory, IPC, synchronization and port rights.
- **BSD subsystem:** POSIX process interfaces, file descriptors, VFS, sockets, networking, credentials and permissions.
- **XNV core:** boot, CPU/platform abstraction, interrupt dispatch, physical memory, paging, system calls and kernel diagnostics.
- **XCM:** first-party userspace system communication and session platform. It connects system services and interfaces to kernel facilities; it is not a kernel subsystem.
- **USI:** future graphical interface built above XCM using the user's programming language. It is deferred until that language, compiler, runtime and standard library are ready.

Mach and BSD are integrated in one privileged kernel address space. XNV must boot and operate without XCM or USI.

## Current XNV milestone

The XNV source tree is under Sources/XNV. The current x86-64 bring-up includes a versioned UEFI boot ABI, UEFI memory-map conversion, ExitBootServices, COM1 diagnostics, GDT/IDT installation, fail-stop exception stubs, a physical page allocator for the low 32 GiB and a 32 GiB identity map using 2 MiB pages. Host regression tests cover the boot ABI and allocator.

This is an early kernel bootstrap, not a complete operating system. The current implementation still assumes UEFI's initial identity mapping while entering the kernel. It does not yet provide a scheduler, userspace, device drivers, storage, networking, a filesystem, a login service or XCM. QEMU boot verification and real hardware testing remain required.

## Transitional Developer Preview

The current installer image still boots the pinned upstream Linux kernel and a Debian Trixie userspace. GRUB, systemd, udev, NetworkManager, GNU utilities, Debian package tooling and external firmware packages remain in that preview. They are temporary migration dependencies and do not satisfy the final first-party system policy.

Do not remove the Linux preview path until XNV has a bootable image and passes automated boot, storage, device, networking, security, installation and recovery tests.

## Implementation languages

The OS foundation uses C, C++ and architecture-specific assembly only. XNV's early kernel is freestanding C and assembly; it must not depend on a host C library, Linux kernel headers or a third-party kernel runtime. C++ is permitted only when exceptions, RTTI, constructors and runtime dependencies are explicitly controlled. Makefiles and CI configuration are build tooling, not OS implementation languages.

## Deferred graphical system

XCM is implemented after the kernel's IPC, process, device, display and input interfaces stabilize. USI begins only after the user's programming language and its toolchain are complete. AppKit, Tarts, GT, Window Server, Finder and Dock remain deferred.
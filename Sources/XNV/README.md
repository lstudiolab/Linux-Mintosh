# XNV kernel

XNV is Mintosh's planned first-party hybrid kernel. It replaces the previous
decision to ship the upstream Linux kernel as Mintosh's permanent kernel.
The current Linux-based Developer Preview remains a temporary bring-up
environment until XNV can boot and pass hardware, storage, networking and
recovery tests. Do not remove the legacy path before that gate is met.

## Architecture

XNV combines two tightly integrated kernel subsystems in one privileged kernel:

- **Mach layer**: task and thread abstractions, scheduling, virtual memory,
  inter-process communication (IPC), synchronization and port rights.
- **BSD layer**: process and POSIX interfaces, VFS, file descriptors, sockets,
  networking, credentials, permissions and compatibility interfaces.
- **XNV core**: boot, CPU/platform abstraction, interrupt and exception
  dispatch, memory ownership, system-call entry, kernel diagnostics and
  subsystem coordination.

This is a Mach/BSD-inspired design, not a copy of Apple's XNU source. The
implementation is first-party C and architecture-specific assembly. C++ may
be used for later kernel components only where its runtime requirements are
explicitly controlled. No host C library, Linux kernel headers, Linux kernel
runtime, or third-party kernel implementation is part of the XNV kernel.

XNV is a **hybrid kernel**, not a userspace microkernel: Mach and BSD services
are integrated in the kernel address space. Drivers may later use isolated
interfaces where practical, but the initial kernel must prioritize a small,
auditable privileged core.

## XCM and USI boundary

XCM is a first-party userspace system-communication and session platform. It
will expose stable IPC, display, input, window, service and session APIs to
higher-level software. It is not a kernel subsystem and must not run with
kernel privilege.

USI is the graphical user interface built above XCM in the user's future
programming language. USI work is explicitly deferred until that language,
compiler, runtime and standard library are ready. XNV must boot, provide a
console and operate without USI or XCM.

## Boot contract v1

`include/xnv/boot.h` defines the versioned bootloader-to-kernel handoff.
All addresses in the structure are physical addresses. The kernel must map
and validate them before dereferencing them. The ABI carries a memory map,
optional initrd, optional ACPI RSDP and optional 32-bit RGBX/BGRX framebuffer.

The current code validates the handoff structure and its ranges. It does not
yet enter long mode, establish page tables, initialize interrupts, schedule
threads, mount a filesystem or boot a machine. The existing mBoot program
still loads the Linux EFI stub; it must be replaced or extended before XNV
can boot.

## Current implementation status

- Implemented: versioned boot handoff ABI and bounds/alignment validation.
- Implemented: host-side regression tests for malformed boot metadata.
- Not implemented: XNV entry/early assembly, GDT/IDT, APIC, physical/virtual
  memory managers, Mach tasks/threads/IPC, BSD process/VFS/socket layers,
  drivers, syscall ABI, scheduler, SMP, userspace ABI or bootable image.
- Not verified: QEMU boot, real hardware boot, suspend/resume, install or
  recovery using XNV.

The ABI is an initial contract and may evolve before the first bootable XNV
release. Changes must be versioned and tested with the bootloader.

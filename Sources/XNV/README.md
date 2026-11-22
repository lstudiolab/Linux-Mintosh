# XNV kernel

XNV is Mintosh's first-party x86-64 hybrid-kernel project, architecturally inspired by Mach and BSD as integrated in Apple's XNU. It is an independent implementation and does not reuse Apple's source.

## Architecture

- **Mach subsystem:** task/thread objects, scheduling, virtual memory, IPC, synchronization and port rights. The core objects and scheduler are not implemented yet.
- **BSD subsystem:** process and POSIX interfaces, VFS, file descriptors, sockets, networking, credentials and permissions. These layers are not implemented yet.
- **XNV core:** boot, CPU/platform abstraction, GDT/IDT, exception dispatch, physical memory, paging and diagnostics.
- **XCM:** future first-party userspace system-communication and session platform. It will expose stable IPC, display, input, window, service and session APIs. It is not part of the kernel.
- **USI:** future graphical interface built above XCM in the user's programming language. It remains deferred until that language, compiler, runtime and standard library are ready.

Mach and BSD are intended to be integrated in one privileged hybrid kernel. XNV must boot and operate without XCM or USI.

## Implemented x86-64 bootstrap

- UEFI x86-64 application entry and ExitBootServices handoff.
- Versioned boot ABI v1 with memory-map, initrd, ACPI RSDP and framebuffer fields.
- UEFI memory descriptor conversion, sorting and validation.
- COM1 serial diagnostics.
- GDT and 256-entry IDT installation.
- Fail-stop CPU exception stubs for vectors 0-31.
- Physical page allocator for the first 32 GiB of physical address space.
- RAM-only identity mapping up to 32 GiB, using 2 MiB pages for complete RAM spans and 4 KiB tables for partial spans. Reserved and MMIO ranges are left unmapped.
- Host regression tests for the boot ABI and physical page allocator.
- Freestanding EFI application build target at out/xnv/XNV.EFI.

The current bootstrap assumes UEFI's initial x86-64 identity mapping while it transitions to its own RAM-only page map. It supports four-level paging and deliberately stops if five-level paging is active. It does not yet establish a higher-half kernel, NX/W^X mappings, per-CPU stacks, APIC routing or SMP.

## Transitional boot dependencies

The current EFI build uses GNU-EFI headers, startup/linker support and libraries to produce the first bootable prototype. This is a temporary bring-up dependency, not the desired final system foundation. A first-party UEFI ABI, entry shim and PE/COFF image pipeline must replace it before a first-party-only XNV release.

The existing Mintosh Developer Preview still boots the upstream Linux kernel and Debian userspace. mBoot still loads the Linux EFI stub in the legacy preview. Do not remove that path until XNV can boot a useful console system and passes installation, storage, device, networking, security and recovery tests.

## Not implemented yet

- APIC/IOAPIC routing, timer interrupts, SMP startup and per-CPU state.
- Mach task/thread objects, context switching, scheduler, IPC ports and VM object model.
- BSD process/syscall ABI, VFS, filesystems, sockets and networking.
- PCI enumeration, driver model, storage, USB and graphics/input drivers.
- User-mode transition, ELF loader, init process, system services and XCM.
- Secure Boot chain, signed kernel image and measured boot.
- Successful OVMF boot verification and physical-hardware validation.

## Build and test

    make xnv-verify
    make xnv-efi
    make test-xnv-qemu

xnv-verify compiles the freestanding core and runs host regression tests. xnv-efi builds the EFI application. test-xnv-qemu boots it under OVMF and checks its serial milestones. A successful compile alone does not prove a working boot.
# Mintosh

Mintosh is an operating-system project targeting the first-party **XNV hybrid kernel**, with Mach-style core kernel architecture and BSD-style Unix functionality. The current Developer Preview still uses Linux and Debian as a temporary bring-up path; it is not the final architecture.

XCM will be the first-party system communication and session layer above XNV. USI will be the graphical interface built on XCM using the user's future programming language. GUI work remains deferred until the language toolchain and system foundation are ready.

## XNV development

The initial XNV x86-64 bootstrap is under Sources/XNV. It currently includes UEFI entry/ExitBootServices, a versioned boot handoff, memory-map conversion, serial diagnostics, GDT/IDT setup, exception stubs, a physical page allocator and an initial identity page map.

On a Debian/Ubuntu x86-64 development host:

    make host-deps
    make xnv-verify
    make xnv-efi

The freestanding EFI kernel image is produced at out/xnv/XNV.EFI. The verification target runs host-side regression tests; a successful compile is not proof of successful firmware or hardware boot.

## Legacy Developer Preview

The current Linux-based preview remains available while XNV is developed:

    make verify
    make kernel-fetch
    make kernel-config
    make kernel
    make iso
    make test-qemu

The bootable preview ISO is out/mintosh-x86_64.iso. It remains a transitional image and is not an XNV release.

## Current limitations

XNV does not yet implement Mach tasks/threads, scheduling, IPC ports, BSD process and syscall interfaces, VFS, filesystems, networking, device drivers, userspace or XCM. Its first memory map and paging implementation is a low-memory bootstrap, not a production virtual-memory system. QEMU firmware boot and physical hardware validation are still required.

The current preview still includes Linux, Debian userspace, GRUB, systemd, udev, NetworkManager, GNU utilities and Debian package tooling. Do not install this Developer Preview on a daily-use machine or overwrite an internal disk.
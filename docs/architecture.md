# Mintosh architecture

## Foundation

Mintosh is a Linux distribution, not a replacement kernel. The upstream Linux kernel provides process scheduling, memory management, drivers, networking, filesystems, and hardware abstraction.

## Planned layers

1. **Boot** — firmware handoff and bootloader configuration.
2. **Kernel** — upstream Linux, configured and built for supported architectures.
3. **Early userspace** — initramfs, device discovery, and mounting the real root filesystem.
4. **Base system** — init/service manager, core utilities, package management, users, permissions, and logging.
5. **Desktop** — display server/compositor, desktop shell, settings, accessibility, and applications.
6. **Distribution tools** — installer, image builder, updates, recovery, and release signing.

## Initial target

The first development target is x86_64 in a virtual machine. AArch64 is an additional build target, not a claim of complete hardware support.

## Current limitations

The repository currently only bootstraps upstream kernel source retrieval and kernel compilation. It does not yet build an initramfs, root filesystem, bootable ISO, installer, or desktop environment.

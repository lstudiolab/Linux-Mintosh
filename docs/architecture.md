# Mintosh architecture

Mintosh is a Linux distribution, not a replacement kernel. The upstream Linux kernel provides scheduling, memory management, drivers, networking, filesystems, and hardware abstraction.

## Base boot sequence

1. GRUB starts the selected Linux kernel and initramfs.
2. The initramfs mounts devtmpfs, procfs, sysfs, and /run.
3. The initramfs launches systemd as PID 1.
4. systemd starts device management, serial login, and systemd-networkd.
5. The Mintosh multi-user target becomes available.

The first image embeds the complete root filesystem in a compressed initramfs. This makes the first ISO straightforward to boot and test without a disk partitioner or installer. The filesystem is RAM-backed and changes are not persistent across reboots. A later disk-installable release will move the root filesystem onto an ext4 partition and use the initramfs only for early hardware discovery and mounting.

## Base userspace

The first root filesystem is bootstrapped from Debian Trixie and includes systemd, GNU core utilities, Bash, util-linux, procps, networking tools, essential shared libraries, sudo, passwd/shadow account tools, and standard administration utilities. Debian is the package source for this bootstrap; Mintosh-specific configuration and identity live in this repository.

## Kernel and releases

Kernel versions are pinned in configs/release.json. Kernel source is downloaded during builds and is not vendored into Git. ARCH=x86_64 maps to the Linux kernel's ARCH=x86; ARCH=arm64 maps to ARCH=arm64. The first complete bootable ISO targets x86_64. ARM64 kernel compilation is supported, while ARM64 rootfs and image packaging are not yet implemented.

## Boot image

The ISO is assembled with GRUB for BIOS and UEFI using grub-mkrescue. The first image is intended for QEMU validation before physical hardware testing.

# Mintosh architecture

Mintosh is a Linux distribution, not a replacement kernel. The upstream Linux kernel provides scheduling, memory management, drivers, networking, filesystems, and hardware abstraction.

## Live boot sequence

1. BIOS or UEFI firmware loads GRUB from the Mintosh USB image.
2. GRUB loads the release-pinned Linux kernel and compressed initramfs.
3. The initramfs mounts devtmpfs, procfs, sysfs, and /run.
4. systemd starts as PID 1.
5. systemd starts device management, networking, the serial console, and the Mintosh installer on tty1.
6. The user can install Mintosh or enter the live shell without modifying internal disks.

## Installed boot sequence

1. Firmware starts GRUB from the installed disk (UEFI EFI System Partition or BIOS boot area).
2. GRUB loads the Mintosh kernel and generated initramfs.
3. The initramfs discovers and mounts the persistent ext4 root partition.
4. systemd starts as PID 1 and brings up the installed system.
5. GRUB's generated menu can include supported existing operating systems detected by os-prober.

## Installation modes

The alongside installer only allocates a new Linux partition inside an existing unallocated free region. It does not shrink, format, or overwrite existing partitions. In UEFI mode it reuses an existing EFI System Partition without formatting it.

The erase-disk installer creates a new GPT partition table. UEFI installations receive an EFI System Partition and ext4 root partition. Legacy BIOS installations receive a BIOS boot partition and ext4 root partition. The installer requires explicit confirmation and typed ERASE before modifying the selected disk.

The first installer is x86_64-only. Automatic partition shrinking, encryption, LVM/RAID, Secure Boot, and ARM64 installation media are not supported.

## Base userspace

The live root filesystem is bootstrapped from Debian Trixie and includes systemd, GNU core utilities, Bash, util-linux, networking tools, essential shared libraries, account tools, disk utilities, and the Mintosh installer. Debian is the package source for this bootstrap; Mintosh-specific configuration and identity live in this repository.

## Kernel and releases

Kernel versions are pinned in configs/release.json. Kernel source is downloaded during builds and is not vendored into Git. ARCH=x86_64 maps to the Linux kernel's ARCH=x86; ARCH=arm64 maps to ARCH=arm64. The first bootable installer targets x86_64.

## Image

The build creates a GRUB BIOS/UEFI hybrid ISO intended to be written to USB media. A source ZIP is not bootable. The complete live root filesystem is embedded in a compressed initramfs, so the live session runs from RAM. The installer copies the base system to a persistent ext4 partition for normal installed use.

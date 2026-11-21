# Installing Mintosh

## Important: source ZIP files are not bootable

Downloading the GitHub repository as a ZIP only gives you source code. A PC firmware/BIOS cannot boot a source ZIP or an extracted source folder. First build the bootable ISO, then write that ISO as a disk image to a USB drive. The ISO contains the GRUB bootloader, Linux kernel, and live Mintosh environment.

## Build the installer USB

On a Debian or Ubuntu x86_64 computer:

    git clone https://github.com/lstudiolab/Linux-Mintosh.git
    cd Linux-Mintosh
    make host-deps
    make iso

The resulting file is:

    out/mintosh-x86_64.iso

Write it to a USB drive using Rufus, balenaEtcher, or GNOME Disks. On Linux, the repository also provides an explicit-confirmation writer:

    make usb USB=/dev/sdX

Replace /dev/sdX with the whole USB device, not a partition such as /dev/sdX1. This operation erases the selected USB drive.

## Boot the live installer

1. Safely eject the USB drive and connect it to the target computer.
2. Open the computer's one-time boot menu (often F12, F9, F8, Esc, or another vendor-specific key).
3. Select the USB device. For a modern Windows installation, choose its UEFI entry.
4. Mintosh starts as a live environment and opens the text-based installer. It does not modify internal disks merely by booting.

## Installation choices

### Install alongside an existing operating system

Mintosh uses only already-unallocated free space on the selected disk. It does not shrink, format, or overwrite existing partitions. Create at least 20 GiB of unallocated space in Windows Disk Management or the existing OS partition manager before starting. On UEFI systems, an existing EFI System Partition is required and is mounted without formatting.

The installer first identifies the ISO9660 volume labeled `MINTOSH` and excludes its parent disk even if firmware reports the USB as non-removable. If it cannot identify the live boot medium, it refuses to display installation disks. It creates a Mintosh ext4 partition in the largest suitable free region, installs GRUB, and enables OS probing so supported existing operating systems (including Windows) can appear in the boot menu.

### Erase disk and install Mintosh

This replaces the selected disk's partition table and contents. The installer displays the exact target disk, requires a warning confirmation, and then requires typing ERASE. All files and operating systems on that disk are permanently lost. Disconnect backup drives and other disks that are not the intended target before using this option.

### Account setup

The installer asks for a username and password. The account is added to the sudo group. Root login remains locked.

## Firmware and hardware notes

- The first installer image targets x86_64 PCs.
- Boot the USB in the same firmware mode as the existing operating system. For Windows installed in UEFI mode, boot Mintosh's UEFI USB entry.
- Secure Boot is not currently supported; disable it in firmware settings before booting the installer.
- Dual boot requires sufficient unallocated space. The installer does not automatically resize Windows partitions.
- GRUB and initramfs tools are bundled in the live image, so installation itself does not require internet access. A network connection is useful after installation.
- Always back up important data before partitioning. Dual-boot and bootloader changes carry risk.

## Current scope

This is a first text-based installer for x86_64. It supports UEFI and legacy BIOS bootloader installation, ext4 root filesystems, UEFI EFI System Partition reuse, and GRUB OS probing. It is not yet a graphical installer, automatic partition shrinker, encrypted-installation tool, RAID/LVM installer, Secure Boot installer, or ARM64 installer.

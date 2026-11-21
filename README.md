# Mintosh

Mintosh is a Linux-based operating system project built around the upstream Linux kernel, systemd, GNU userspace, and a custom installation flow.

## Current milestone: bootable Mintosh installer

The x86_64 image now builds a live Mintosh environment with a text-based installer. It supports:

- GRUB boot from BIOS and UEFI USB media.
- Live boot without modifying internal disks.
- Install alongside an existing OS using already-unallocated disk space.
- Erase a selected disk after two explicit confirmations.
- Persistent ext4 root filesystem.
- UEFI EFI System Partition reuse for dual boot, without formatting it.
- GRUB installation and OS probing for supported existing systems, including Windows.
- User account and password setup.
- QEMU serial-console boot smoke test.

## Build the installer

On a Debian/Ubuntu x86_64 build host:

    git clone https://github.com/lstudiolab/Linux-Mintosh.git
    cd Linux-Mintosh
    make host-deps
    make kernel-fetch
    make kernel-config
    make kernel
    make iso

The installer image is created at:

    out/mintosh-x86_64.iso

Write the ISO to a USB drive using Rufus, balenaEtcher, or GNOME Disks. On Linux, an explicit-confirmation writer is included:

    make usb USB=/dev/sdX

Use the whole USB device (for example /dev/sdb), not a partition (for example /dev/sdb1). Writing the image erases that USB drive.

**A GitHub source ZIP is not bootable.** BIOS/UEFI firmware cannot boot the repository ZIP or an extracted source folder. Build the ISO and flash it to USB first. See docs/installation.md.

## Installation modes

- **Alongside:** uses only an existing unallocated region of at least 20 GiB. It does not resize or format existing partitions. UEFI dual boot reuses the existing EFI System Partition.
- **Erase disk:** replaces the selected disk's partition table and data. The installer requires a warning acknowledgement and typing ERASE before it proceeds.

Back up important data before installation. For dual boot, create unallocated space from the existing OS's disk manager before booting Mintosh.

## Build and test targets

Run make help for all targets. Important targets:

- make kernel-fetch
- make kernel-config
- make kernel
- make iso
- make qemu
- make test-qemu
- make usb USB=/dev/sdX

The automated GitHub Actions workflow builds the image, boots it in QEMU, and uploads the ISO and serial log as workflow artifacts.

## Architecture

See docs/architecture.md, docs/development.md, and docs/installation.md.

## Current limitations

The installer currently targets x86_64 only. It uses a text interface, requires internet during installation for bootloader package setup, and expects pre-existing unallocated space for dual boot. Automatic Windows partition shrinking, disk encryption, LVM/RAID, Secure Boot, graphical desktop, application store, and ARM64 installer media are not implemented. Secure Boot must be disabled for this initial image.

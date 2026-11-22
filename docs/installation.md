# Installing Mintosh

## Installer package and USB preparation

A GitHub source ZIP is not bootable. Build or obtain the official Mintosh installer ZIP, extract it, and write the included ISO to a USB drive.

To package a locally built ISO:

    make package-installer

The output is:

    out/mintosh-installer.zip

It contains:
- `mintosh-x86_64.iso` — the bootable installer image.
- `SHA256SUMS` — the ISO checksum.
- `INSTALL.txt` — Rufus instructions and workspace requirements.

On Windows, extract the ZIP, open Rufus, select `mintosh-x86_64.iso`, and write it to the USB drive. Rufus will erase the selected USB. A 4 GB USB is the design target; the ISO build rejects images larger than 3.5 GiB. Verify the ISO with `sha256sum --check SHA256SUMS` on Linux before writing it.

## Source-first installation flow

The USB contains a small live bootstrap environment, not the full Mintosh installation. The complete Mintosh root filesystem is built on the target laptop.

1. Boot the USB in UEFI mode and choose **Install Mintosh**.
2. Connect to the internet using Wi-Fi or Ethernet. If needed, the installer opens the NetworkManager text interface.
3. Choose an existing ext4 partition for a temporary build workspace. It must have at least 40 GiB free. The installer mounts it without formatting and creates a hidden temporary directory.
4. The installer retrieves the exact Git commit embedded in the ISO. It does not silently substitute a moving `main` branch revision.
5. The laptop runs source verification, hardware checks, kernel configuration/build, root-filesystem creation, ISO assembly, and a QEMU boot test.
6. The native hardware audit reports 150 checks. Required platform/build checks must pass. Optional hardware features such as TPM, Bluetooth, battery telemetry and vendor-specific interfaces may be reported as SKIP when absent; TPM is not an installation requirement.
7. Only after successful verification does the installer display the installation choices and target disks.
8. The verified locally built root filesystem is copied to the selected target, GRUB is installed, and the installed build records its source commit in `/etc/mintosh/build-info`.

The installer requires a working internet connection. Source retrieval and all kernel/rootfs compilation happen on the target laptop, not on GitHub Actions.

## Temporary workspace requirements

The live environment is RAM-backed, so it cannot safely hold a full Linux kernel build in memory. A persistent ext4 workspace with at least 40 GiB free is required.

- The workspace may be an internal partition or a separate external SSD.
- The installer never formats the workspace partition.
- The USB boot medium is protected and cannot be selected as the workspace.
- If the workspace and installation target are on the same physical disk, **alongside** installation is allowed only into already-unallocated space.
- **Erase disk** is blocked when the selected target is the workspace disk, because erasing it would destroy the verified build. Use a separate workspace drive for an erase install.
- Failed checks leave the target disk untouched. The installer displays a warning and requests a return to UEFI firmware setup.

## Installation choices

### Install alongside an existing operating system

Mintosh uses only already-unallocated free space on the selected disk. It does not shrink, format, or overwrite existing partitions. Create at least 20 GiB of unallocated space before starting. On UEFI systems, an existing EFI System Partition is required and is mounted without formatting.

The installer identifies the live boot medium and excludes its parent disk. If it cannot identify the live medium, it refuses to display installation disks. It creates a Mintosh ext4 partition in the largest suitable free region and preserves existing partitions.

### Erase disk and install Mintosh

This replaces the selected disk's partition table and contents. The target disk must be at least 24 GiB. The installer checks capacity, displays the exact target disk, requires a warning confirmation, and then requires typing `ERASE`. All files and operating systems on that disk are permanently lost. The workspace disk cannot be selected for this operation.

### Account setup

The installer asks for a username and password. The account is added to the sudo group. Root login remains locked.

## Firmware and security notes

- The first installer image targets x86_64 UEFI PCs. Legacy BIOS boot remains available through GRUB for existing compatibility.
- Secure Boot signing and key enrollment are not implemented. Disable Secure Boot before booting this Developer Preview.
- TPM 2.0 is optional. Measured boot is not implemented yet.
- The 150-point audit is a preflight, not a guarantee of support for every laptop. Hardware-specific driver support still requires real-machine testing.
- mBoot is not yet the active ISO or installed-system bootloader; GRUB remains active until mBoot passes firmware runtime, kernel/initramfs handoff and installer integration tests.
- Always back up important data before partitioning. Dual-boot and bootloader changes carry risk.

## Current scope

This is a source-first text installer for x86_64. It builds the target kernel and root filesystem locally, verifies the generated ISO in QEMU, supports UEFI and legacy BIOS GRUB installation, ext4 root filesystems, UEFI EFI System Partition reuse, and alongside installation into existing free space. It is not yet a graphical installer, automatic partition shrinker, encrypted-installation tool, signed Secure Boot installer, TPM measured-boot implementation, RAID/LVM installer, or ARM64 installer.

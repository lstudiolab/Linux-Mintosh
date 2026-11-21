# Mintosh Base development

## Host requirements

A Debian- or Ubuntu-based x86_64 build host with internet access, sudo, and enough free disk space for a kernel build and live root filesystem (allow at least 25 GB).

Install dependencies:

    make host-deps

## Build the bootable installer

    make kernel-fetch
    make kernel-config
    make kernel
    make iso

The resulting installer image is out/mintosh-x86_64.iso. It includes the live system and text-based installer.

## Run in QEMU

Interactive boot:

    make qemu

Automated serial-console smoke test:

    make test-qemu

The test passes only when the systemd-managed Mintosh ready service prints its boot marker. The serial log is saved to out/qemu-boot.log.

## Write installer USB

Use Rufus, balenaEtcher, or GNOME Disks to write the ISO to USB. On Linux:

    make usb USB=/dev/sdX

The whole selected USB device is overwritten. The writer requires typing the exact target path to confirm.

## Build stages

- make kernel-fetch: download the pinned upstream Linux source.
- make kernel-config: configure kernel support for initramfs, devtmpfs, ext4, virtio, serial console, EFI, and FAT.
- make kernel: compile the kernel and modules.
- make rootfs: bootstrap Debian Trixie, install the live installer and disk tools, and configure networking.
- make kernel-install: install the versioned kernel and modules into the root filesystem.
- make initramfs: package the live root filesystem into a compressed cpio initramfs.
- make iso: assemble the BIOS/UEFI GRUB boot image.
- make test-qemu: verify the live environment reaches the Mintosh ready marker.

## Installer behavior

The alongside path uses only a pre-existing unallocated region of at least 20 GiB. It does not resize or format existing partitions. UEFI installs reuse the EFI System Partition. The erase path replaces the selected disk's partition table after a warning and typed ERASE confirmation. Both modes install GRUB and generate an OS-probing boot menu.

The installer is x86_64-only. Secure Boot is not supported. Automatic partition shrinking, encryption, LVM/RAID, and ARM64 installer media are not implemented.

## ARM64 kernel build

The kernel can also be cross-compiled for ARM64 from an x86_64 host:

    make ARCH=arm64 kernel-fetch
    make ARCH=arm64 kernel-config
    make ARCH=arm64 kernel

ARM64 uses a separate output directory and the aarch64-linux-gnu cross-compiler. The live installer, root filesystem, and ISO packaging currently remain x86_64-only.

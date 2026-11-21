# Mintosh

Mintosh is a Linux-based operating system built around the upstream Linux kernel, systemd, GNU userspace and a custom installation flow. The project aims for a consistent, minimalist desktop experience while retaining Linux hardware and application compatibility.

## Current release

The release manifest is the source of truth:

    make check-release
    make release-info

The first Developer Preview targets x86_64 bootable media. ARM64 is supported by the kernel build configuration; ARM64 live images and installation are not yet implemented.

## Build the x86_64 installer

On a Debian/Ubuntu x86_64 build host with internet access and sudo:

    git clone https://github.com/lstudiolab/Linux-Mintosh.git
    cd Linux-Mintosh
    make host-deps
    make verify
    make kernel-fetch
    make kernel-config
    make kernel
    make iso

The ISO is created at:

    out/mintosh-x86_64.iso

Run it interactively:

    make qemu

Run the serial-console smoke test:

    make test-qemu

The automated GitHub Actions workflow performs the same validation and build, runs QEMU, and uploads the ISO and serial log.

## Build stages

- `make check-release`: validate the release manifest.
- `make verify`: validate release metadata and shell syntax.
- `make host-deps`: install Debian/Ubuntu build dependencies.
- `make kernel-fetch`: download and verify the pinned upstream Linux source.
- `make kernel-config`: configure the kernel for Mintosh and QEMU.
- `make kernel`: compile the kernel and modules.
- `make rootfs`: bootstrap the Debian-based Mintosh Base.
- `make initramfs`: assemble the live initramfs.
- `make iso`: create BIOS/UEFI GRUB installer media.
- `make test-qemu`: verify that systemd reaches the Mintosh ready service.
- `make usb USB=/dev/sdX`: write the ISO to a USB drive after explicit confirmation.

A GitHub source ZIP is not bootable. Firmware needs the built ISO written to a USB device.

## Installation

The text installer supports installation into existing unallocated space or erasing a selected disk after two explicit confirmations. The alongside flow does not shrink or format existing partitions. Back up important data before installing.

See [installation](docs/installation.md), [architecture](docs/architecture.md), [development](docs/development.md), [release management](docs/release-management.md) and the [roadmap](docs/roadmap.md).

## Current limitations

The desktop, graphical application catalog, Mintosh update service, Secure Boot, disk encryption, automatic partition resizing and ARM64 installer are not implemented yet. The QEMU boot script and CI workflow exist; a successful run is required before calling a release boot-verified.

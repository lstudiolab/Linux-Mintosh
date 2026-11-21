# Mintosh

Mintosh is a Linux-based operating system built around the upstream Linux kernel and systemd. Mintosh-owned native userspace components are developed under `Sources/` and installed into the base image.

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

The ISO is created at `out/mintosh-x86_64.iso`.

Run it interactively with `make qemu`, or run the serial-console boot test with `make test-qemu`.

## Native Mintosh source

`Sources/` contains the native source for Darl, launch7762, launch511, LibSys, CSF, Foundation, I/O+, GVD, FMS, DM, IOPS, Kexts, Security, and System Logging. CSF exposes typed system status/version and protected service-control APIs for future native applications. The target root filesystem compiles these programs inside its own Debian userspace and installs them into the image.

After boot, use:

    mintoshctl status
    mstore search browser
    sudo mstore install firefox
    mintoshctl version
    mintoshctl services
    ioplus
    gvdctl list
    iopsctl status
    dmctl list
    fmsctl index /home
    fmsctl search documents
    kextctl list
    security-audit

`make verify` also compiles the native core and runs its CLI smoke tests.

## Build stages

- `make check-release`: validate the release manifest.
- `make verify`: validate release metadata, shell syntax, and native source build/tests.
- `make host-deps`: install Debian/Ubuntu build dependencies.
- `make kernel-fetch`: download and verify the pinned upstream Linux source.
- `make kernel-config`: configure the kernel for Mintosh.
- `make kernel`: compile the kernel and modules.
- `make rootfs`: bootstrap the Debian-based base and compile/install native Mintosh components.
- `make iso`: create BIOS/UEFI GRUB installer media.
- `make test-qemu`: verify the live environment reaches the Mintosh readiness check.
- `make usb USB=/dev/sdX`: write the ISO to a USB drive after explicit confirmation.

A GitHub source ZIP is not bootable. Firmware needs the built ISO written to a USB device.

## Installation

The text installer supports installation into existing unallocated space or erasing a selected disk after two explicit confirmations. The alongside flow does not shrink or format existing partitions. Back up important data before installing.

See [installation](docs/installation.md), [architecture](docs/architecture.md), [development](docs/development.md), [release management](docs/release-management.md) and the [roadmap](docs/roadmap.md).

## Current limitations

The graphical desktop (AppKit, Tarts, GT, Window Server, Dock and Finder), graphical application catalog, Mintosh update service, Secure Boot, disk encryption, automatic partition resizing and ARM64 installer are not implemented yet. The QEMU boot script and CI workflow exist; a successful run is required before calling a release boot-verified.

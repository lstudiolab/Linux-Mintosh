# Mintosh

Mintosh is a Linux-based operating system built around the upstream Linux kernel and systemd. Mintosh-owned native userspace components are developed under `Sources/` and installed into the base image. The native x86-64 UEFI bootloader, mBoot, is developed under `Sources/mBoot/`.

## Current release

The release manifest is the source of truth:

    make check-release
    make release-info

The first Developer Preview targets x86_64 bootable media. ARM64 can be cross-compiled at the kernel stage; ARM64 root filesystems, live images and installation are not implemented. RISC-V is planned.

**Hardware safety:** this is still a Developer Preview, not a hardware-certified installer. Do not install it on your daily-use laptop or overwrite an internal disk yet. Require a successful QEMU boot test and a disposable-disk installation test before physical installation.

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

Network setup is provided by NetworkManager. Ethernet connects automatically when DHCP is available. For Wi-Fi, use `nmcli device wifi list` and `nmcli --ask device wifi connect "NETWORK_NAME"`.

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
    sudo fmsctl index /home
    fmsctl search documents
    kextctl status
    kextctl list
    sudo security-audit

`make verify` also validates required boot/module-signing kernel options, compiles the native core, runs regression/CLI smoke tests, verifies live boot-media exclusion and tests alongside partition preservation plus confirmation-gated UEFI and BIOS erase layouts. The manual GitHub Actions workflow **Mintosh QEMU boot verification** builds the pinned kernel and ISO, then boots the image under QEMU.

## Build stages

- `make check-release`: validate the release manifest.
- `make verify`: validate release metadata, shell syntax, and native source build/tests.
- `make host-deps`: install Debian/Ubuntu build dependencies.
- `make kernel-fetch`: download and verify the pinned upstream Linux source.
- `make kernel-config`: configure the kernel for Mintosh and verify boot-critical, virtualization and enforced module-signing options.
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

mBoot currently builds as a standalone x86-64 UEFI application, but it is not yet wired into the ISO's firmware boot path; GRUB remains the active BIOS/UEFI loader. OVMF runtime boot tests, firmware Secure Boot signing, TPM measured boot and kernel/initramfs measurement are pending. The graphical desktop (AppKit, Tarts, GT, Window Server, Dock and Finder), graphical application catalog, Mintosh update service, disk encryption, automatic partition resizing and ARM64 installer are also not implemented yet. The QEMU boot script and manual CI workflow exist, but a successful run on the current release commit is still required before calling the image boot-verified.

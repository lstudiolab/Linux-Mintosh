# Mintosh

Mintosh is a Linux-based operating system project. It uses the upstream Linux kernel and is being developed into an installable desktop operating system.

## Current milestone: Mintosh Base

The repository now contains the first complete base-system build pipeline:

- Release-pinned upstream Linux kernel source and x86_64 kernel configuration.
- Debian Trixie root filesystem with systemd as PID 1.
- GNU core utilities, Bash, util-linux, procps, essential libraries, and administration tools.
- systemd-networkd DHCP configuration and systemd-resolved.
- Local user and sudo-group creation with locked-by-default credentials.
- A full-rootfs initramfs, GRUB BIOS/UEFI ISO, and QEMU boot smoke test.

The first image is a RAM-backed live boot image. It is a bootable base-system milestone, not yet a disk installer or desktop release.

## Quick start

On a Debian/Ubuntu x86_64 build host:

    make host-deps
    export MINTOSH_PASSWORD='choose-a-local-password'
    make kernel-fetch
    make kernel-config
    make kernel
    make iso
    make test-qemu

The image is written to out/mintosh-x86_64.iso. Run it interactively with make qemu.

If MINTOSH_PASSWORD is omitted, the mintosh account is created with a locked password. Root login is locked.

## Build targets

Run make help for the full target list. The main stages are kernel-fetch, kernel-config, kernel, rootfs, initramfs, iso, qemu, and test-qemu.

## Architecture

See docs/architecture.md for the boot sequence and system layers, and docs/development.md for build requirements and limitations.

## Kernel source

The upstream Linux source is fetched at build time using the version in configs/release.json. It is not vendored into this repository. The source is GPL-2.0-only; see the kernel source tree's COPYING file and the Linux kernel licensing documentation.

## Current limitations

The initial image embeds its complete root filesystem in initramfs, so changes are lost on reboot. The project does not yet include a graphical desktop, persistent disk image, installer, package manager frontend, update framework, secure-boot signing, or ARM64 image builder. Validate in QEMU before attempting physical hardware.

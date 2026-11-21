# Mintosh Base development

## Host requirements

A Debian- or Ubuntu-based x86_64 build host with internet access, sudo, and enough free disk space for a kernel build and root filesystem (allow at least 25 GB).

Install dependencies:

    make host-deps

## Build the bootable image

Optionally set the initial account password. If omitted, the mintosh account is created locked.

    export MINTOSH_PASSWORD='choose-a-local-password'
    make kernel-fetch
    make kernel-config
    make kernel
    make iso

The resulting image is out/mintosh-x86_64.iso.

## Run in QEMU

Interactive boot:

    make qemu

Automated serial-console smoke test:

    make test-qemu

The test passes only when the systemd-managed mintosh-ready.service prints its boot marker. The serial log is saved to out/qemu-boot.log.

## Build stages

- make kernel-fetch: download the pinned upstream Linux source.
- make kernel-config: configure kernel support for initramfs, devtmpfs, ext4, virtio, serial console, and EFI.
- make kernel: compile the kernel and modules.
- make rootfs: bootstrap Debian Trixie and configure Mintosh identity, user accounts, permissions, and DHCP networking.
- make initramfs: embed the root filesystem in a compressed cpio initramfs.
- make iso: assemble the BIOS/UEFI GRUB boot image.
- make test-qemu: verify that systemd reaches the Mintosh ready marker.

## Important limitations

This is an initial live boot image, not an installer or finished desktop OS. The current initramfs contains the entire root filesystem in RAM; changes do not persist after reboot. There is no graphical desktop, package manager frontend, disk installer, update framework, secure boot signing, or ARM64 image builder yet. Do not install this image on a daily-use machine.

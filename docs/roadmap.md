# Mintosh development roadmap

This roadmap distinguishes code that exists from milestones that have passed a real boot test. A script or configuration file is not considered verified merely because it has been committed.

## Release 0.1.0 — Developer Preview

### Release and build infrastructure
- [x] Semantic release version in `configs/release.json`
- [x] Exact upstream Linux version and SHA-256 pin
- [x] Build-time kernel fetch; kernel source is not vendored
- [x] Separate kernel build output by architecture
- [x] x86_64 and ARM64 kernel build mappings
- [x] Local release-manifest validation
- [ ] Reproducible-build verification across clean hosts

### Mintosh Base
- [x] Debian Trixie bootstrap
- [x] systemd as PID 1
- [x] GNU core utilities and Bash
- [x] Essential shared libraries and account tools
- [x] systemd-networkd DHCP configuration
- [x] sudo user creation and root-account lock
- [x] Linux kernel modules and ext4 support
- [ ] Hardware matrix beyond QEMU
- [ ] Network smoke test in CI

### Boot and installer
- [x] Custom initramfs assembly
- [x] GRUB BIOS/UEFI ISO assembly
- [x] Live environment and text installer
- [x] Persistent ext4 installation flow
- [x] Existing EFI System Partition reuse for UEFI dual boot
- [x] Explicit confirmation for erase-disk installation
- [x] QEMU serial-console smoke-test script
- [ ] Passing QEMU boot test on the release commit
- [ ] Physical-machine installation test
- [ ] ARM64 bootable image and installer

## Later releases

### Mintosh Desktop
- [ ] Select and integrate a Wayland compositor
- [ ] Display manager and graphical session
- [ ] Menu bar and system status area
- [ ] Dock and application launcher
- [ ] File manager (Finder-style)
- [ ] System Settings
- [ ] Notifications and session services
- [ ] Accessibility, HiDPI, keyboard and input support

### Applications and updates
- [ ] PackageKit integration
- [ ] Graphical application catalog using distribution package metadata
- [ ] Application install, remove and update flows
- [ ] Signed Mintosh release metadata
- [ ] Atomic or rollback-capable system update strategy
- [ ] Kernel update and boot-entry management

### Installer and release engineering
- [x] BIOS/UEFI ISO build pipeline
- [ ] Graphical installation wizard
- [ ] Partition resize support with safety checks
- [ ] Optional disk encryption
- [ ] Secure Boot signing
- [ ] ARM64 installer media
- [ ] Published release checksums and signed release artifacts

## Architecture rule

Mintosh is a Linux distribution. Linux remains the kernel and provides hardware drivers, scheduling, memory management, filesystems and networking. Mintosh owns the userspace integration, system services, desktop experience, installer and release tooling. Kernel source is downloaded at build time from the exact version and checksum in the release manifest.

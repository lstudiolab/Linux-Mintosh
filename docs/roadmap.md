# Mintosh development roadmap

This roadmap distinguishes source implementation from release verification. A directory or build target is not considered complete until its implementation and tests exist.

## Release 0.1.0 — Developer Preview

### Release and build infrastructure
- [x] Semantic release version and pinned Linux source
- [x] Separate kernel build output by architecture
- [x] x86_64 and ARM64 kernel build mappings
- [x] Local release-manifest validation
- [ ] Reproducible-build verification across clean hosts

### Mintosh-owned native source
- [x] LibSys shared POSIX utility library
- [x] MOSF local foundation daemon and Unix-socket API
- [x] launch7762 native child-process service supervisor
- [x] launch511 command execution helper
- [x] CSF client library
- [x] Foundation string and path primitives
- [x] I/O+ hardware inventory over sysfs/procfs
- [x] Kexts kernel-module control frontend
- [x] System Logging syslog frontend
- [x] Security basic critical-file permission audit
- [ ] AppKit native GUI framework
- [ ] Tarts widget toolkit
- [ ] GT graphics backend and BlueE compositor
- [ ] Finder and Dock applications

### Mintosh Base
- [x] Debian Trixie bootstrap
- [x] systemd as PID 1
- [x] GNU core utilities and Bash
- [x] Native Mintosh programs compiled in the target root filesystem
- [x] Native service manager and MOSF started during boot
- [x] systemd-networkd DHCP configuration
- [x] sudo user creation and root-account lock
- [ ] Hardware matrix beyond QEMU
- [ ] Network smoke test in CI

### Boot and installer
- [x] Custom initramfs assembly
- [x] GRUB BIOS/UEFI ISO assembly
- [x] Live environment and text installer
- [ ] Passing QEMU boot test on the release commit
- [ ] Physical-machine installation test
- [ ] ARM64 bootable image and installer

## Later releases

### Mintosh Desktop
- [ ] Wayland compositor and display manager
- [ ] Menu bar and system status area
- [ ] Dock and application launcher
- [ ] Finder-style file manager
- [ ] System Settings and notifications
- [ ] Accessibility, HiDPI, keyboard and input support

### Applications and updates
- [ ] PackageKit integration and application catalog
- [ ] Application install, remove and update flows
- [ ] Signed Mintosh release metadata
- [ ] Rollback-capable system updates
- [ ] Kernel update and boot-entry management

### Installer and release engineering
- [ ] Graphical installation wizard
- [ ] Partition resize support with safety checks
- [ ] Optional disk encryption
- [ ] Secure Boot signing
- [ ] ARM64 installer media
- [ ] Published release checksums and signed artifacts

## Architecture rule

Linux remains the kernel and provides drivers, scheduling, memory management, filesystems and networking. Mintosh owns the native userspace services, frameworks, desktop experience, installer and release tooling.

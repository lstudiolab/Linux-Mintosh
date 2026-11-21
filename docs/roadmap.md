# Mintosh development roadmap

This roadmap separates implemented source from verified release behavior. A directory, binary, or successful compilation is not proof that a subsystem is complete.

## Release 0.1.0 — Developer Preview

### Release and build infrastructure
- [x] Semantic release version and pinned Linux source
- [x] Separate kernel build output by architecture
- [x] x86_64 and ARM64 kernel build mappings
- [x] Local release-manifest validation
- [ ] Reproducible-build verification across clean hosts
- [ ] ARM64 root filesystem, boot image and installer

### Mintosh-owned native source
- [~] LibSys POSIX utilities (filesystem, descriptors, Unix sockets, runtime paths and syslog; broader API and tests remain)
- [~] Darl local daemon (basic health/version protocol; authorization, structured requests and durable state remain)
- [~] launch7762 native service supervisor (process lifecycle, protected definitions and restart backoff; per-service users, dependencies and resource limits remain)
- [~] launch511 command execution helper (direct argv execution; policy and privilege model remain)
- [~] CSF client library (Darl request transport only; higher-level system APIs remain)
- [~] Foundation library (strings, byte buffers, arrays, dictionaries, URL parsing, errors, lexical paths and bounded atomic file I/O; date/time, localization, serialization and broader filesystem APIs remain)
- [~] I/O+ inventory (structured sysfs device properties and kernel uevent stream; driver capability probing and stable device identity remain)
- [~] HP ProBook compatibility baseline (multi-generation x86-64 driver profile, vendor firmware packages and DMI diagnostics; model-by-model hardware certification remains)
- [~] Kexts Linux module frontend (list/load/unload wrapper; signed module policy and lifecycle integration remain)
- [~] System Logging syslog frontend (write path exists; structured querying, rotation policy and retention remain)
- [~] Security baseline audit (critical account files, protected service definitions and installed executable ownership/modes; policy engine, capabilities and sandboxing remain)
- [~] mstore APT frontend (Debian signed repository transport; Mintosh catalog, transaction UI and rollback remain)
- [ ] Mintosh-signed package repository and curated catalog
- [ ] AppKit native GUI framework
- [ ] Tarts widget toolkit
- [ ] GT graphics backend and Window Server compositor
- [ ] Finder and Dock applications

### Mintosh Base
- [x] Debian Trixie bootstrap
- [x] systemd as PID 1
- [x] GNU core utilities and Bash
- [x] Native Mintosh programs compiled in the target root filesystem
- [x] Native service manager and Darl started during boot
- [x] systemd-networkd DHCP configuration
- [x] sudo user creation and root-account lock
- [~] HP ProBook multi-generation driver and firmware baseline
- [ ] Hardware matrix beyond QEMU
- [ ] Network smoke test in CI
- [ ] End-to-end service control and package transaction tests in installed rootfs

### Boot and installer
- [x] Custom initramfs assembly
- [x] GRUB BIOS/UEFI ISO assembly
- [x] Live environment and text installer
- [x] Passing QEMU boot test on the release commit
- [ ] Installer destructive-operation test suite
- [ ] Physical-machine installation test on representative HP ProBook Intel and AMD models
- [ ] ARM64 bootable image and installer

## Foundation completion gates

The graphical desktop does not start until the following are complete:

1. Every native component has a documented API, explicit ownership and privilege model, and unit/integration tests.
2. Service supervision has safe argv execution, peer-credential authorization, bounded restart behavior, clean shutdown and rootfs integration.
3. Package operations have transaction validation, trusted repository configuration, failure recovery and tests.
4. Security checks cover account files, service definitions, executable ownership, module loading and privileged IPC.
5. I/O+ reports structured hardware data and handles device changes without requiring a reboot.
6. The installer passes destructive-path tests in disposable virtual disks, preserves existing partitions in alongside mode, and verifies firmware boot entries before reboot.
7. QEMU boot, network and service tests pass for each advertised architecture.

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
- [ ] Published release checksums and signed artifacts

## Architecture rule

Linux remains the kernel and provides drivers, scheduling, memory management, filesystems and networking. Mintosh owns the native userspace services, frameworks, desktop experience, installer and release tooling.

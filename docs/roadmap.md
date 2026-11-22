# Mintosh development roadmap

This roadmap separates implemented source from verified release behavior. A directory, binary, or successful compilation is not proof that a subsystem is complete.

## Release 0.1.0 — Developer Preview

### Release and build infrastructure
- [x] Semantic release version and pinned Linux source
- [x] Separate kernel build output by architecture
- [x] x86_64 and ARM64 kernel build mappings
- [x] Local release-manifest validation
- [~] Native foundation verification workflow (workflow added; successful hosted run still needs confirmation)
- [~] Manual QEMU boot workflow (pinned kernel/ISO build and QEMU test defined; successful run still needs confirmation)
- [ ] Reproducible-build verification across clean hosts
- [ ] ARM64 root filesystem, boot image and installer

### Mintosh-owned native source
- [~] LibSys POSIX utilities (filesystem, descriptors, Unix sockets, runtime paths and syslog; broader API and tests remain)
- [~] Darl local daemon (bounded health/version protocol and service-group socket permissions; structured IPC, authorization scopes and durable state remain)
- [~] launch7762 system supervisor and Launch Client per-user supervisor (protected definitions, direct argv execution, peer credentials, bounded restart and clean shutdown; dependency graphs and resource limits remain)
- [~] launch511 command execution helper (direct argv execution; policy and privilege model remain)
- [~] CSF system framework (Darl status/version and launch7762 service list/control APIs; structured IPC, notifications and asynchronous requests remain)
- [~] Foundation library (strings, byte buffers, arrays, dictionaries, URL parsing, errors, lexical paths and bounded atomic file I/O; date/time, localization, serialization and broader filesystem APIs remain)
- [~] GVD task management and dispatch (bounded worker queue, process inventory, nice values and non-realtime scheduler policies; cancellation, cgroup resource controls and cross-process task policy remain)
- [~] IOPS power services (battery/thermal telemetry, CPU governor and platform profile controls; vendor-specific policy daemon, suspend integration and power event API remain)
- [~] I/O+ inventory and kernel uevent monitor (structured device data and live hotplug events; driver capability probing and stable device identity remain)
- [~] FMS metadata search (atomic SQLite file index and bounded search; incremental indexing, content search and query service remain)
- [~] DM disk manager (sysfs disk inventory and restricted mount/unmount operations; filesystem health, encryption and partition planning remain)
- [~] Generic x86-64 Intel/AMD hardware baseline (ACPI, common storage, USB, wired/wireless networking, Bluetooth, graphics, audio and input drivers; hardware-matrix validation remains)
- [~] Optional HP ProBook profile (vendor-specific drivers and DMI diagnostics; model-by-model hardware certification remains)
- [~] Kexts Linux module frontend (safe module names, fixed-path modprobe and kernel-enforced signatures; trusted third-party signing and lifecycle integration remain)
- [~] System Logging syslog frontend (write path exists; structured querying, rotation policy and retention remain)
- [~] Security baseline audit (account files, service definitions, runtime sockets, FMS database permissions and enforced module signatures; policy engine, capabilities and sandboxing remain)
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
- [~] Generic x86-64 hardware profile is the default; HP ProBook driver profile is opt-in
- [ ] Hardware matrix beyond QEMU
- [ ] Network smoke test in CI
- [ ] End-to-end service control and package transaction tests in installed rootfs

### Boot and installer
- [x] Custom initramfs assembly
- [x] GRUB BIOS/UEFI ISO assembly
- [x] Live environment and text installer
- [ ] QEMU boot test on the current release commit (must pass before physical installation is considered safe)
- [~] Installer storage safety tests (live ISO parent disk exclusion, alongside partition preservation, exact ERASE confirmation and UEFI/BIOS partition layouts are tested; filesystem formatting and full install recovery tests remain pending)
- [ ] Physical-machine installation test on representative HP ProBook Intel and AMD models
- [ ] ARM64 bootable image and installer

## Foundation completion gates

The graphical desktop does not start until the following are complete:

1. Every native component has a documented API, explicit ownership and privilege model, and unit/integration tests.
2. Service supervision has safe argv execution, peer-credential authorization, bounded restart behavior, clean shutdown and rootfs integration.
3. Package operations have transaction validation, trusted repository configuration, failure recovery and tests.
4. Security checks cover account files, service definitions, executable ownership, module loading and privileged IPC.
5. I/O+ reports structured hardware data and handles device changes without requiring a reboot; GVD, FMS, DM and IOPS expose tested native APIs with explicit privilege boundaries.
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

## Architecture and language rules

Linux remains the kernel and provides drivers, scheduling, memory management, filesystems and networking. Mintosh-owned native system components use C, C++ where structured userspace components benefit from it, and architecture-specific assembly for CPU/boot primitives. Build orchestration may use Make and CI configuration; shell-based runtime installer code is technical debt to migrate, not a completed native implementation. Mintosh owns the native userspace services, frameworks, desktop experience, installer and release tooling.

Architecture status is explicit: x86_64 is the only bootable installer target; ARM64 is kernel-buildable only; RISC-V is planned. An architecture is not advertised as supported until its root filesystem, boot image, installer and CI boot tests are complete.

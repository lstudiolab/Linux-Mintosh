# Mintosh architecture

## First-party system policy

The upstream Linux kernel is the sole third-party software component permitted in the installed Mintosh operating system. Mintosh owns the bootloader, init process, service supervisor, device-management userspace, networking services, storage tools, security policy, logging, installer, recovery environment, package manager and all system frameworks. These components must be implemented in the repository using C, C++ and assembly. Standard kernel interfaces and open hardware protocols are interfaces, not bundled third-party implementations.

No GRUB, systemd, udev, NetworkManager, APT/dpkg, Flatpak, RPM tooling, BusyBox, GNU userland suite or other external operating-system utility may be a runtime dependency. Vendor firmware blobs are also outside the permitted software set unless explicitly approved as a documented hardware exception. Build-host compilers, linkers, Make, GitHub Actions and QEMU may be used as development/test tools; they must not be copied into the installed system as substitutes for Mintosh components.

**Migration status:** the current preview image does not yet comply with this target. It is still Debian-bootstrapped and currently uses GRUB, systemd, udev, NetworkManager, GNU utilities, Debian package tooling and third-party firmware packages. Existing native Mintosh programs are incremental work, not evidence that the userspace has already been replaced. The foundation is not complete or ready for graphical development until these dependencies have been removed from the shipped image and the first-party replacements pass boot and integration tests.

Mintosh is a Linux distribution, not a replacement kernel. The upstream Linux kernel provides scheduling, memory management, drivers, networking, filesystems, and hardware abstraction.

## Live boot sequence

1. BIOS or UEFI firmware loads GRUB from the Mintosh USB image.
2. GRUB loads the release-pinned Linux kernel and compressed initramfs.
3. The initramfs mounts devtmpfs, procfs, sysfs, and /run.
4. systemd starts as PID 1.
5. systemd starts device management, networking, the serial console, launch7762, and the Mintosh installer on tty1.
6. launch7762 starts enabled Mintosh-managed services, including Darl.

## Mintosh-owned native core

Native source code lives under `Sources/`. The current core is compiled inside the Debian target root filesystem, so its binaries link against the target userspace rather than the build host.

- **Mintosh Init (prototype)**: first-party PID 1 process with kernel pseudo-filesystem setup, launch7762 supervision, orphan reaping, bounded restart backoff and orderly supervisor shutdown. It is built and regression-tested but is not yet selected by the installed boot path and does not yet provide a login/session manager.
- **Darl**: local Unix-socket system foundation API for status and version queries.
- **launch7762 / Launch Background**: system-wide native service supervisor. It reads protected `/etc/mintosh/services/*.service` definitions, starts enabled processes with direct `execve`, tracks child exits, applies capped exponential restart backoff and authorizes control operations with Unix peer credentials.
- **Launch Client**: per-user background service supervisor with user-owned service definitions and a private user control socket.
- **launch511**: standalone command execution helper. launch7762 executes validated service argument vectors directly and does not route service commands through a shell.
- **LibSys**: shared POSIX utilities, bounded Unix-socket line framing with deadlines, file-descriptor helpers, runtime paths and syslog integration.
- **CSF**: C client API for the Darl local service endpoint.
- **Foundation**: C17 strings, byte data, arrays, dictionaries, URL parsing, error values, lexical paths and bounded atomic file I/O.
- **GVD**: process scheduling inspection/control plus a bounded pthread task-dispatch library for native userspace.
- **IOPS**: power-supply and thermal inventory, CPU frequency governor inspection/control and ACPI platform-profile management.
- **I/O+**: structured kernel, CPU, memory, DMI, block, network, USB, graphics and power inventory, plus a read-only kernel uevent monitor.
- **Platform baseline**: x86-64 builds enable a generic Intel/AMD desktop and laptop driver profile covering ACPI, CPU power management, PCI storage, NVMe, SATA/AHCI, USB, common wired and wireless networking, Bluetooth, graphics, audio, input, webcams and Thunderbolt/USB-C. Intel Wi-Fi, Realtek RTL8821CE/RTL8822BE, Realtek wired Ethernet, Intel graphics and common storage drivers are explicitly included as modules. Device-specific drivers remain modules so missing hardware does not block boot.
- **HP Compatibility**: C++ platform diagnostics backed by an x86-64 assembly CPUID routine, plus DMI-based HP/ProBook identification. The HP-specific kernel profile is opt-in with `MINTOSH_HARDWARE_PROFILE=hp-probook-450-g6` (the older `hp-probook` profile name remains an alias). It enables HP WMI/hotkey and accelerometer support plus the Intel 9560 and Realtek 8821CE/8822BE WLAN alternatives documented for the ProBook 450 G6. The generic profile remains the default for other x86-64 computers.
- **Kexts**: kernel module operations delegated to Linux modprobe/kmod.
- **Security**: basic permissions audit for critical account files.
- **System Logging**: syslog client utility.

systemd remains PID 1 in this release and supervises launch7762. The new Mintosh Init binary is an isolated, tested implementation milestone only; it is not yet safe to select as the default PID 1 because console sessions, networking startup, recovery behavior and complete boot integration are unfinished. launch7762 supervises Mintosh-managed services. This is an incremental architecture: replacing PID 1 is a separate boot-critical project and is not claimed by this release.

## Installed boot sequence

1. Firmware starts GRUB from the installed disk.
2. GRUB loads the Mintosh kernel and generated initramfs.
3. The initramfs discovers and mounts the persistent ext4 root partition.
4. systemd starts as PID 1 and brings up the installed system.
5. GRUB's generated menu can include supported existing operating systems detected by os-prober.

## Installation modes

The alongside installer only allocates a new Linux partition inside an existing unallocated free region. It does not shrink, format, or overwrite existing partitions. In UEFI mode it reuses an existing EFI System Partition without formatting it.

The erase-disk installer creates a new GPT partition table. UEFI installations receive an EFI System Partition and ext4 root partition. Legacy BIOS installations receive a BIOS boot partition and ext4 root partition. The installer requires explicit confirmation and typed ERASE before modifying the selected disk.

The first installer is x86_64-only. Automatic partition shrinking, encryption, LVM/RAID, Secure Boot, and ARM64 installation media are not supported.

## Implementation languages

Mintosh-owned native system components use C for portable low-level interfaces and core services, C++ where structured userspace system components benefit from it, and assembly only for architecture-specific CPU/boot primitives. Architecture-specific code belongs under an architecture directory and must not leak into portable interfaces. The upstream Linux kernel is implemented in C and assembly and remains responsible for privileged hardware access and device drivers. Mintosh userspace must use kernel interfaces such as sysfs, ioctl, udev and device nodes rather than executing privileged CPU instructions. Makefiles, GitHub Actions and host-side packaging scripts are build/release tooling, not kernel or native-system implementation code; the live installer and remaining shell-based runtime helpers are still migration work and are not represented as completed native C/C++/assembly components.

## Base userspace

The current preview root filesystem is Debian Trixie-based and includes systemd, GNU core utilities, Bash, util-linux, NetworkManager, udev and other external utilities. This is a temporary development bootstrap and explicitly violates the first-party system policy above. It must be replaced with a Mintosh-built root filesystem containing only the Linux kernel and Mintosh-owned userspace. Do not describe the present rootfs as a completed or compliant Mintosh base system.

## Kernel and releases

Kernel versions are pinned in configs/release.json. Kernel source is downloaded during builds and is not vendored into Git. ARCH=x86_64 maps to the Linux kernel's ARCH=x86; ARCH=arm64 maps to ARCH=arm64. The verified bootable installer target is x86_64. Generic Intel/AMD hardware support is the default kernel profile; an HP ProBook 450 G6 overlay is optional and can be selected with `make kernel-config MINTOSH_HARDWARE_PROFILE=hp-probook-450-g6`. The QEMU workflow configures and validates both profiles before building the generic live image. ARM64 can be kernel-cross-compiled but has no root filesystem, installer or bootable image yet. RISC-V is planned, not implemented. The Debian Trixie root filesystem enables non-free-firmware and installs firmware for common Intel, AMD, Realtek, Atheros, Broadcom, MediaTek and NVIDIA devices.

## Image

The build creates a GRUB BIOS/UEFI hybrid ISO intended to be written to USB media. A source ZIP is not bootable. The complete live root filesystem is embedded in a compressed initramfs, so the live session runs from RAM. The installer copies the base system to a persistent ext4 partition for normal installed use.

## Package management

`mstore` is the first-party package-manager interface. Its former APT/dpkg/Flatpak command dispatch has been removed. The current CLI can identify package extensions and directly launch a user-selected AppImage, but native package search, metadata, database, installation, removal, repository updates, dependency resolution, signature verification and rollback are not implemented. Installation is disabled until the native transaction engine is ready.

Mintosh's only native installable format is **`.pkg`**, with a Mintosh-owned manifest, payload format, dependency model, permission policy, lifecycle metadata, signature format, package database and transactional installer. Linux package formats may be identified, but they are not runtime dependencies and must not be installed by delegating to their ecosystem's package managers. Any future import compatibility must be implemented by Mintosh-owned code and remain optional.

## Developer tools

A Mintosh SDK and an Xcode-like integrated development environment are planned, but explicitly deferred. Work on them will begin only after the system foundation is stable and the user's programming language, compiler, runtime and standard library are ready. The eventual toolchain should support building, testing, signing and publishing native `.pkg` applications, while making supported Linux development workflows accessible to developers.

## Development phase policy

The system foundation is the active development phase and must be completed before graphical-system work begins. Foundation completion means the machine can boot, initialize hardware, manage devices and storage, establish networking, supervise services, enforce security boundaries, install and update packages safely, log and recover from failures, and operate reliably without a graphical session. These capabilities require documented interfaces and passing automated integration and boot tests; compiling individual binaries is not sufficient.

The user's programming language is the next prerequisite. Its compiler, runtime, standard library and development tools must be complete enough to implement production system software before AppKit or any graphical subsystem is started. AppKit, the graphics backend, window server, desktop shell and native graphical applications will then be implemented in that language.

This policy intentionally defers all graphical interface and AppKit implementation. It does not change the current Linux-kernel-based architecture or the C/C++/assembly implementation rules for the present system foundation.

## Graphical stack

AppKit, Tarts, GT, Window Server, Finder and Dock are not implemented yet. They must be real graphical components with defined rendering, input, accessibility and application-lifecycle interfaces; the current native core does not pretend to provide them.

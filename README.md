# Mintosh

Mintosh is a Linux-based operating system project. It uses the upstream Linux kernel and will grow into a complete, installable desktop operating system.

## Project status

This repository is the starting point. It currently contains the project structure and scripts to obtain and build a real Linux kernel. It is **not yet a bootable operating system**.

## Goals

- Linux kernel as the foundation; do not write a new kernel.
- A small, maintainable userspace and clear separation between system components.
- Support for x86_64 first, with AArch64 as a planned target.
- Reproducible build steps and documented dependencies.
- A consistent, minimalist Mintosh identity.

## Repository layout

```text
Mintosh/
├── boot/                 # Bootloader integration (planned)
├── configs/              # Kernel and system build configuration
├── docs/                 # Architecture and development notes
├── kernel/               # Upstream Linux kernel workspace (downloaded)
├── rootfs/               # Mintosh root filesystem (planned)
├── scripts/              # Build and development scripts
├── .gitignore
├── Makefile
└── README.md
```

The upstream kernel source is downloaded into `kernel/linux/` when requested. It is not copied into this repository: the Linux source tree is very large and has its own release cadence and GPL-2.0 license.

## Requirements

For an x86_64 Linux build host:

- Git
- curl
- Python 3
- xz
- make
- GCC or Clang and the standard build tools
- Linux kernel build dependencies (including flex, bison, bc, openssl development headers, and elfutils)

Install the build dependencies using your host distribution's package manager.

## Quick start

```sh
make help
make kernel-fetch
make kernel-config
make kernel
```

The first build uses the kernel's default configuration for the selected architecture. Build output is written under `out/kernel/`.

Select an architecture with:

```sh
make ARCH=x86_64 kernel-fetch
make ARCH=arm64 kernel-fetch
```

## Important

Building the kernel alone does not produce a usable operating system. Mintosh still needs a bootloader, an init system, a root filesystem, device management, networking, graphics, audio, installer, and update system. Those will be added as separate, testable components.

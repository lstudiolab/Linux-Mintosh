# Native Mintosh source tree

This directory contains Mintosh-owned userspace code. It is compiled for the target root filesystem during `make rootfs`; it is not copied into the kernel source tree.

## Implemented components

| Component | Source | Executable/library | Responsibility |
|---|---|---|---|
| MOSF | `MOSF/main.c` | `mosf` | Local system-facility status/version socket |
| launch7762 | `Launch7762/main.c` | `launch7762` | Config-driven process supervisor |
| launch511 | `Launch511/main.c` | `launch511` | Command execution helper |
| LibSys | `LibSys/` | `libsys.h` | Unix sockets, filesystem helpers, syslog |
| CSF | `CSF/` | `libcsf.a` | C client for MOSF |
| Foundation | `Foundation/` | `libfoundation.a` | String ownership and path utilities |
| I/O+ | `IOPlus/main.c` | `ioplus` | sysfs/procfs hardware inventory |
| Kexts | `Kexts/main.c` | `kextctl` | Linux kernel module operations through kmod |
| Security | `Security/main.c` | `security-audit` | Critical account-file permission checks |
| System Logging | `SystemLogging/main.c` | `mosflog` | syslog event submission |
| Mintosh control | `MintoshCtl/main.c` | `mintoshctl` | MOSF and launch7762 client |

## Build and test

    make -C Sources/MintoshCore clean all
    sh Sources/MintoshCore/tests/smoke.sh

The Debian target root filesystem is built with the same Makefile through chroot. This ensures generated executables use the target system's libc and linker environment.

## Runtime endpoints

- MOSF: `/run/mintosh/mosf.sock` (read-only status/version commands)
- launch7762: `/run/mintosh/launch.sock` (privileged service controls)
- Service definitions: `/etc/mintosh/services/*.service`

A service definition supports `ExecStart`, `Enabled=yes|no`, and `Restart=always|on-failure|no`.

## Deliberate boundaries

This is the first native userspace layer, not a completed desktop or a replacement for systemd. systemd remains PID 1 and supervises launch7762. AppKit, Tarts, GT, BlueE, Finder, Dock, audio, display management and the remaining named frameworks are not represented by empty stubs; they require their own real implementations and tests.

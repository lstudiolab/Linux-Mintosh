# Lenix upstream source integration

This document records the trusted upstream projects selected for Lenix and how their code is used. The source list is machine-readable in [../configs/upstream-sources.json](../configs/upstream-sources.json).

## Integration rules

- Linux is the only kernel. XCU builds and configures upstream Linux; it does not contain a separately booted Mach or FreeBSD kernel.
- Prefer upstream Linux implementations for kernel functionality already provided by Linux. The pinned Linux source fetched by `make kernel-fetch` already contains the selected IPC, Unix-socket and device-core files.
- Use FreeBSD source to validate BSD/POSIX behavior and compatibility expectations. FreeBSD kernel implementation files are not drop-in Linux kernel code.
- Keep Mach-inspired task, port and memory-object APIs as Lenix-owned abstractions over Linux pidfds, AF_UNIX/SOCK_SEQPACKET, SCM_RIGHTS, memfd seals and epoll.
- Linux owns kernel drivers and the device model. I/O Kit is an architectural reference only; do not attempt to compile Apple's I/O Kit or transplant its kernel classes.
- Record source URL, exact file path, blob SHA and applicable license before copying or adapting upstream code.
- Never remove upstream copyright notices or SPDX identifiers. Check every file's license before redistributing modifications.

## Source-to-component mapping

| Upstream | Lenix component | Integration decision |
|---|---|---|
| Linux `ipc/msg.c` | XCU | Already supplied by the pinned Linux kernel; enables System V IPC in the XCU config |
| Linux `net/unix/af_unix.c` | XCU / XUM transport | Linux Unix sockets underpin local message transport |
| Linux `drivers/base/core.c` | XCU device/driver layer | Linux device core remains authoritative |
| FreeBSD `uipc_syscalls.c`, `uipc_mqueue.c`, `sysv_msg.c`, `if.c` | BSD/POSIX compatibility | Behavioral reference; Linux kernel interfaces remain the implementation |
| U-Boot `common/main.c`, `bootm.c`, `drivers/core/device.c` | Boot research | Not used by the current GRUB/mBoot boot path |
| systemd `src/core/main.c`, `manager.c` | Sean service lifecycle | Current Debian preview uses systemd; Lenix-native service manager remains first-party |
| Wayland `wayland-server.c`, `wayland-client.c` | Future Window Server | Protocol and implementation reference; not yet linked |
| Mesa upstream | Cacao graphics | Official freedesktop Mesa source for future OpenGL/Vulkan support |

## XCU kernel configuration

The XCU fragment enables Linux's existing Unix-domain socket and event facilities, POSIX message queues, and System V IPC. These are kernel capabilities, not copied FreeBSD code. The kernel-config verifier checks that required options remain enabled.

## What is and is not integrated

**Active in the current Linux/XCU build path:**
- Linux IPC, Unix sockets, descriptor passing and device core, through the pinned upstream kernel.
- XCU's Lenix-owned pidfd task handles, sequenced-packet ports, sealed memory objects and epoll event queues.
- Linux POSIX/System V IPC configuration, including message queues.

**Reference-only or future work:**
- FreeBSD source is not compiled into XCU.
- U-Boot is not part of the current GRUB/mBoot boot path.
- systemd is a transitional runtime dependency, not copied into Lenix's native service manager.
- Wayland and Mesa are not yet part of a completed graphical build. Window Server and Cacao remain separate implementation work.

No Apple source or Apple-specific kernel code is used.

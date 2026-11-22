# XNV — Linux-backed kernel architecture

XNV is the kernel architecture and first-party integration layer inside Darkin. Linux is the one actual kernel. XNV does not boot a separate BSD or Mach kernel.

## Kernel responsibilities

Linux owns CPU scheduling, process execution, virtual memory, VFS, networking, device drivers, interrupts, kernel security and hardware control.

The XNV Linux configuration enables the facilities required by the architecture:

- Namespaces and cgroups for process and resource isolation.
- seccomp, Linux Security Framework hooks, audit and keys for security policy.
- epoll, eventfd, signalfd, timerfd and POSIX message queues for event and IPC primitives.
- Unix sockets and descriptor passing for port-like message channels.
- pidfds for stable process/task references.
- memfd and file seals for shareable, bounded memory objects.
- inotify/fanotify for filesystem observation.

## First-party XNV API

The C17 library in this directory wraps Linux primitives behind stable Mintosh interfaces:

| XNV abstraction | Linux implementation |
|---|---|
| Task handle | pidfd and pidfd_send_signal |
| Port pair | AF_UNIX SOCK_SEQPACKET and SCM_RIGHTS |
| Memory object | sealable memfd |
| Event queue | epoll |

These are Mach-inspired abstractions, not a Mach kernel. They preserve Linux as the scheduler, VM and security authority.

## BSD-derived compatibility

Linux already provides the Unix/POSIX and BSD-derived socket interface used by Mintosh. XNV uses native Linux socket, file-descriptor and event APIs rather than embedding a BSD kernel or maintaining a redundant compatibility implementation. Any additional BSD-specific API compatibility must be added only when a concrete userspace requirement exists.

## Build and tests

    make -C Sources/Darkin/XNV
    make -C Sources/Darkin/XNV test

The root filesystem build installs `libxnv.a` and `xnv.h`. Host tests cover pidfd task handles, sequenced message ports, descriptor passing, sealed memory objects and epoll event delivery.

This is a Linux userspace integration library. It does not modify the upstream Linux kernel source or claim that the full Darkin kernel policy layer is complete.

# XCU Linux kernel integration

XCU is Mintosh's Linux-based hybrid-kernel architecture. Linux is the actual and only primary kernel.

Linux owns scheduling, virtual memory, VFS, networking, drivers, interrupts, hardware support and kernel security. XCU adds Mintosh-specific integration and user-space abstractions.

- BSD-derived Unix/POSIX compatibility is implemented using Linux interfaces; no second BSD kernel is embedded.
- Mach-inspired task, port, message-rights and memory-object abstractions use Linux pidfds, Unix sequenced-packet sockets, SCM_RIGHTS, sealed memfd objects and epoll.
- Tarts owns the XCU integration and complete backend foundation.
- XPU provides the separate authenticated IPC and security broker for Cacao and Tarts services.

The configuration fragment enables namespaces, cgroups, IPC/event primitives, filesystem observation, seccomp, audit, BPF and required platform facilities. It is a kernel baseline, not proof that every target feature or security policy is implemented.

The previous `kernel/xnv/` path is retained for migration compatibility. New kernel builds use `kernel/xcu/config.fragment`.

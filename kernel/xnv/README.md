# XCU Linux integration

XCU is Mintosh's kernel architecture built around the Linux kernel as its primary execution kernel.

Linux remains responsible for scheduling, virtual memory, VFS, networking, device drivers, interrupts, hardware support and kernel security primitives. XCU adds first-party Mintosh integration.

- BSD-derived compatibility uses the Unix/POSIX, socket, file-descriptor and event interfaces already provided by Linux. No FreeBSD kernel is embedded.
- Mach-inspired task, port-like IPC, message-rights and memory-object concepts use Linux pidfds, Unix sequenced-packet sockets, SCM_RIGHTS, memfd seals and epoll. Linux remains the scheduler and VM authority.
- Tarts owns XCU integration and the backend foundation.
- XPU is the separate communication/security broker used by Cacao and Tarts services.
- Cacao is the graphical platform and native application framework; Window Server is a separate graphical system service.

The kernel configuration fragment captures Linux namespaces, cgroups, IPC/event, filesystem observation, seccomp, audit and BPF facilities required by XCU. It is a starting configuration, not proof that every future kernel policy feature is implemented.

The first-party userspace primitive wrappers and tests are exposed through the canonical `Sources/XCU/` build entry point. Their implementation is temporarily retained in `Sources/Darkin/XNV/` pending tested source migration.

# XNV Linux integration

XNV is Mintosh's kernel architecture built around the Linux kernel as its primary execution kernel.

Linux remains responsible for scheduling, virtual memory, VFS, networking, device drivers, interrupts, hardware support and kernel security primitives. XNV adds first-party Mintosh integration.

- BSD-derived compatibility uses the Unix/POSIX, socket, file-descriptor and event interfaces already provided by Linux. No FreeBSD kernel is embedded.
- Mach-inspired task, port-like IPC, message-rights and memory-object concepts use Linux pidfds, Unix sequenced-packet sockets, SCM_RIGHTS, memfd seals and epoll. Linux remains the scheduler and VM authority.
- Darkin owns XNV and all backend services.
- XCM is outside Darkin and Choca and mediates Choca requests.
- Choca is outside Darkin and owns the graphical platform.

The kernel configuration fragment captures Linux namespaces, cgroups, IPC/event, filesystem observation, seccomp, audit and BPF facilities required by XNV. It is a starting configuration, not proof that every future kernel policy feature is implemented.

The first-party userspace primitive wrappers and tests live in `Sources/Darkin/XNV`.

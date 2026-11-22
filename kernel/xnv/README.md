# XNV Linux integration

XNV is the Mintosh kernel architecture built around the Linux kernel as its primary execution kernel.

Linux remains responsible for scheduling, virtual memory, VFS, networking, device drivers, interrupts, hardware support and kernel security primitives. XNV adds first-party Mintosh integration.

- BSD-derived compatibility interfaces are implemented by Mintosh on Linux facilities; no FreeBSD kernel is embedded.
- Mach-inspired task, port-like IPC, message-rights and memory-object concepts are implemented by Mintosh using Linux primitives; Linux remains the scheduler and VM authority.
- Darkin owns XNV and all backend services.
- XCM is outside Darkin and mediates Choca requests.
- Choca is outside Darkin and owns the graphical platform.

The kernel config fragment in this directory captures baseline Linux facilities needed by the XNV architecture. It is a starting point, not proof that the BSD/Mach-inspired interfaces have been implemented.

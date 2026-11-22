# Linux kernel within XNV

Linux is the actual primary kernel used by Mintosh. XNV is Mintosh's integrated kernel architecture and first-party policy layer built on Linux; it is not a second kernel.

Linux supplies the scheduler, virtual memory, VFS, networking, device model, drivers, interrupt handling, hardware support and kernel security primitives. XNV adds first-party Mintosh integration and policy on top.

BSD-derived APIs and compatibility behavior are implemented as first-party layers using Linux facilities. Mach-inspired tasks, port-like IPC, capability rights and message abstractions are likewise implemented using Linux facilities. Mintosh does not embed a separate FreeBSD kernel or copy Apple's XNU.

The current preview already boots Linux. Its Debian userspace and system services are transitional and will be replaced by Darkin-owned backend services over time.

The standalone EFI experiment in `Sources/XNV` is not the selected kernel. It is retained as a reference prototype only; do not use its EFI image for Mintosh boot.

Kernel source license: GPL-2.0-only. See the source tree's COPYING file and https://www.kernel.org/doc/html/latest/process/license-rules.html.

# Linux kernel within XNV

Linux is the actual primary kernel used by Lenix. XNV is Lenix's integrated kernel architecture and first-party policy layer built on Linux; it is not a second kernel.

Linux supplies the scheduler, virtual memory, VFS, networking, device model, drivers, interrupt handling, hardware support and kernel security primitives. XNV adds first-party Lenix integration and policy on top.

BSD-derived APIs and compatibility behavior use the Unix/POSIX, socket, file-descriptor and event facilities already provided by Linux. Mach-inspired tasks, port-like IPC, capability rights and memory-object concepts are implemented with Linux pidfds, Unix sequenced-packet sockets, descriptor passing, memfd seals and epoll. Lenix does not embed a separate FreeBSD kernel or copy Apple's XNU.

The current preview boots upstream Linux. Its Debian userspace and system services are transitional and will be replaced by Sean-owned backend services over time.

The standalone EFI experiment in `Sources/XNV` is not the selected kernel. It is retained as a reference prototype only; do not use its EFI image for Lenix boot.

Kernel source license: GPL-2.0-only. See the source tree's COPYING file and https://www.kernel.org/doc/html/latest/process/license-rules.html.

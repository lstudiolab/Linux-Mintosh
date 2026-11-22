# XCU

XCU is the single kernel architecture of Mintosh and the rebranded continuation of XNV. Do not create or maintain a parallel XNV kernel.

Linux is the actual kernel. XCU integrates and configures Linux, then provides Mintosh-owned kernel-facing interfaces and abstractions:
- Linux process, scheduling, virtual memory, VFS, networking and driver facilities remain authoritative.
- BSD-derived interfaces provide Unix/POSIX compatibility on Linux.
- Mach-inspired task, port, message and memory-object concepts are implemented using Linux primitives.
- Linux modules and drivers provide hardware extension points.

XCU is not a custom standalone kernel and does not boot alongside Linux. The abandoned standalone EFI experiment formerly under `Sources/XNV/` is not part of the selected architecture. Mintosh boots the Linux kernel through its boot manager.

The initial primitive implementation currently remains under `Sources/Darkin/XNV/`; it is migration debt and must be moved into this canonical source root. The public API is `libxcu.a` and `<xcu/xcu.h>`. New code must not introduce XNV names or include paths.

Run `make -C Sources/XCU test` to build and execute the current Linux-backed primitive tests. A passing userspace test does not validate Linux kernel configuration, hardware drivers or a bootable release.

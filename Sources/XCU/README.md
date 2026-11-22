# XCU

XCU is the single kernel architecture of Mintosh and the rebranded continuation of XNV. There is no parallel XNV kernel.

Linux is the actual kernel. XCU integrates and configures Linux, then provides Mintosh-owned kernel-facing interfaces and abstractions:
- Linux process, scheduling, virtual memory, VFS, networking and driver facilities remain authoritative.
- BSD-derived interfaces provide Unix/POSIX compatibility on Linux.
- Mach-inspired task, port, message and memory-object concepts are implemented using Linux primitives.
- Linux modules and drivers provide hardware extension points.

XCU is not a custom standalone kernel and does not boot alongside Linux. The old standalone EFI experiment in `Sources/XNV/` is retired; the selected boot path loads the Linux kernel.

## Canonical implementation

The canonical C17 implementation now lives in this directory:
- `Primitives/task.c`: pidfd-backed task handles.
- `Primitives/port.c`: sequenced-packet IPC and descriptor passing.
- `Primitives/memory.c`: sealed memfd memory objects.
- `Primitives/event.c`: epoll event queues.
- `include/xcu/xcu.h`: public XCU API.

Builds produce `libxcu.a`; new code must include `<xcu/xcu.h>` and use `xcu_*` symbols. Legacy XNV source files remain only as migration references and are not part of the canonical XCU build.

Run `make -C Sources/XCU test` to build and execute the canonical Linux-backed primitive tests. A passing userspace test does not validate Linux kernel configuration, hardware drivers or a bootable release.

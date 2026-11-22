# XCU

XCU is Lenix's single Linux-based kernel architecture.

Linux is the actual kernel. XCU integrates and configures Linux, then provides Lenix-owned kernel-facing interfaces and abstractions:
- Linux process scheduling, virtual memory, VFS, networking, drivers, interrupts, hardware support and kernel security remain authoritative.
- BSD-derived interfaces provide Unix/POSIX compatibility using Linux facilities. Linux System V IPC and POSIX message queues are enabled in the XCU kernel configuration.
- Mach-inspired task, port, message and memory-object concepts are implemented using Linux pidfds, AF_UNIX/SOCK_SEQPACKET, SCM_RIGHTS, sealed memfd objects and epoll.
- Linux modules and drivers provide hardware extension points. The Linux device core remains authoritative; I/O Kit is an architectural reference, not imported source.

## Canonical implementation

The canonical C17 implementation lives in this directory:
- `Primitives/task.c`: pidfd-backed task handles.
- `Primitives/port.c`: sequenced-packet IPC and descriptor passing.
- `Primitives/memory.c`: sealed memfd memory objects.
- `Primitives/event.c`: epoll event queues.
- `include/xcu/xcu.h`: public XCU API.

Builds produce `libxcu.a`; new code must include `<xcu/xcu.h>` and use `xcu_*` symbols. Legacy XNV source files remain migration references and are not part of the canonical XCU build.

## Upstream integration

The upstream source map and exact retrieved file revisions are recorded in [docs/upstream-integration.md](../../../docs/upstream-integration.md) and [configs/upstream-sources.json](../../../configs/upstream-sources.json).

Linux kernel files are supplied by the pinned Linux source fetched through `scripts/fetch-kernel.sh`; they are not duplicated here. FreeBSD sources are behavioral references because FreeBSD kernel internals cannot be dropped into Linux. See the source map for licenses and integration decisions.

Run `make -C Sources/XCU test` to build and execute the Linux-backed primitive tests. A passing userspace test does not validate kernel configuration, hardware drivers or a bootable release.

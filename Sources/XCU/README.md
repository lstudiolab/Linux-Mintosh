# XCU Kernel integration

XCU is Mintosh's Linux-based kernel architecture. Linux remains the actual primary kernel; BSD-derived Unix compatibility and Mach-inspired task/port/message abstractions are implemented over Linux.

The first userspace primitive library is retained in `../Darkin/XNV/` pending migration. It wraps Linux pidfds, Unix sequenced-packet sockets with descriptor passing, sealed memfd objects and epoll event queues.

The selected kernel is upstream Linux plus `kernel/xnv/` integration. The standalone EFI experiment in `../XNV/` is not the selected kernel.

Run `make -C Sources/XCU test` to run the current Linux primitive tests. These userspace wrappers are not a replacement kernel.

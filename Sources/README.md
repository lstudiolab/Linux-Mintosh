# Mintosh native source tree

Mintosh-owned native userspace is implemented in ISO C17. Every source file is named for its responsibility. Executable entry points are separated from protocol handling, process management, filesystem operations, hardware discovery and reusable library code.

## Source layout

- `Darl/`: system foundation daemon and local protocol.
- `Launch7762/`: service configuration, process lifecycle, restart policy, control protocol and Unix socket.
- `Launch511/`: command argument handling and process execution.
- `LaunchClient/`: per-user service supervisor entry point.
- `LibSys/`: POSIX filesystem, descriptors, Unix sockets, line framing, runtime paths and system logging.
- `CSF/`: C client API for Darl.
- `Foundation/`: strings, byte buffers, arrays, dictionaries, URL parsing, error values, lexical paths and bounded atomic file I/O.
- `GVD/`: Linux process scheduling controls and a bounded native worker-queue library.
- `IOPS/`: power-supply, thermal, CPU governor and platform-profile inspection/control.
- `IOPlus/`: kernel, memory and sysfs hardware inventory.
- `HPCompatibility/`: HP/ProBook identity and driver diagnostics.
- `Kexts/`: Linux kernel module operations through kmod.
- `Security/`: baseline permission and service-definition audit.
- `SystemLogging/`: journald/syslog integration.
- `MintoshCtl/`: system and service control client.
- `PackageManager/`: native mstore interface to signed Debian APT repositories.

## Build and test

    make -C Sources/MintoshCore clean all test-foundation

    sh Sources/MintoshCore/tests/smoke.sh

The smoke suite compiles native components, exercises the Darl and service-manager IPC paths, validates GVD concurrency and IOPS policy parsing, starts and stops child processes, and tests Foundation, filesystem, package-validation and security boundaries. Tests use disposable temporary directories.

See [the C17 implementation standard](../docs/c-language-standard.md).

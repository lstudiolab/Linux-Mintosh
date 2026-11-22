# Mintosh native source tree

Mintosh's native source is implemented in C17, C++ where appropriate, and architecture-specific assembly. The final kernel is XNV and is freestanding C/assembly. Existing POSIX/Linux-dependent programs are preview userspace prototypes, not the final XNV userspace ABI.

## Source layout

- XNV/: first-party hybrid kernel bootstrap, boot ABI, memory, paging, interrupt and architecture code.
- Darl/: system foundation daemon and local protocol.
- Launch7762/: service configuration, process lifecycle, restart policy, control protocol and Unix socket.
- Launch511/: command argument handling and process execution.
- LaunchClient/: per-user service supervisor entry point.
- LibSys/: POSIX filesystem, descriptors, Unix sockets, line framing, runtime paths and logging.
- CSF/: C client API for Darl.
- Foundation/: strings, byte buffers, arrays, dictionaries, URL parsing, errors, paths and bounded atomic file I/O.
- GVD/: process inspection/control and native worker-queue library.
- IOPS/: power and thermal inventory and controls.
- IOPlus/: Linux preview hardware inventory and uevent monitoring.
- HPCompatibility/: HP/ProBook diagnostics.
- Kexts/: legacy Linux module operations through kmod.
- Security/: baseline permissions audit.
- SystemLogging/: current journald/syslog integration.

## Build and test

    make -C Sources/MintoshCore clean all test-foundation
    sh Sources/MintoshCore/tests/smoke.sh
    make xnv-verify

See docs/c-language-standard.md.
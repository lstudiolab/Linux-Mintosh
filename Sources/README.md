# Mintosh native source tree

Every source file is named for the responsibility it implements. Executable entry points are separated from protocol handling, process management, filesystem operations, hardware discovery and reusable library code.

## Source layout

- `MOSF/`: `mosf_daemon.c` owns the daemon lifecycle; `mosf_protocol.c` implements its request/response protocol.
- `Launch7762/`: service configuration parsing, process lifecycle, control protocol, Unix socket and daemon entry point are separate translation units.
- `Launch511/`: command argument handling and process execution are separate.
- `LibSys/`: filesystem, file-descriptor I/O, Unix-domain socket transport and system logging.
- `CSF/`: client request API for MOSF.
- `Foundation/`: owned string values and path operations.
- `IOPlus/`: kernel information, sysfs inventory, memory information and command entry point.
- `Kexts/`: kernel module command execution and CLI.
- `Security/`: file permission inspection and audit command.
- `SystemLogging/`: log writer and CLI.
- `MintoshCtl/`: MOSF client, service-manager client and CLI.

## Build and test

    make -C Sources/MintoshCore clean all
    sh Sources/MintoshCore/tests/smoke.sh

The smoke test compiles every component, checks executable entry points and exercises MOSF through its real Unix-domain socket API.

## Runtime interfaces

- MOSF: `/run/mintosh/mosf.sock`; supports `PING`, `VERSION` and `STATUS`.
- launch7762: `/run/mintosh/launch.sock`; supports `LIST`, `START NAME`, `STOP NAME` and `RESTART NAME`.
- Service definitions: `/etc/mintosh/services/*.service`; fields are `ExecStart`, `Enabled` and `Restart`.

systemd remains PID 1 in this release and supervises launch7762. launch7762 supervises Mintosh-managed services. Linux module loading is delegated to the Linux kernel's kmod tooling; this source does not claim to implement kernel drivers.

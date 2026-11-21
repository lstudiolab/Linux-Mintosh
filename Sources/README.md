# Mintosh native source tree

Every source file is named for the responsibility it implements. Executable entry points are separated from protocol handling, process management, filesystem operations, hardware discovery and reusable library code.

## Source layout

- `MOSF/`: daemon lifecycle and request/response protocol.
- `Launch7762/`: service configuration parsing, process lifecycle, restart policy, control protocol, Unix socket and daemon entry point.
- `Launch511/`: command argument handling and process execution.
- `LibSys/`: filesystem, file-descriptor I/O, Unix-domain socket transport, runtime paths and system logging.
- `CSF/`: client request API for MOSF.
- `Foundation/`: reusable string and path primitives.
- `IOPlus/`: kernel information, sysfs inventory, memory information and command entry point.
- `Kexts/`: Linux kernel module operations through kmod.
- `Security/`: file permission inspection and audit command.
- `SystemLogging/`: log writer and CLI.
- `MintoshCtl/`: MOSF client, service-manager client and CLI.

## Build and test

    make -C Sources/MintoshCore clean all
    sh Sources/MintoshCore/tests/smoke.sh

The smoke test compiles every component, exercises MOSF over its Unix-domain socket, and starts/stops real child processes through launch7762. It verifies failure-based restart behavior and rejects group/world-writable service definitions. Tests use temporary runtime and service directories; production defaults remain `/run/mintosh` and `/etc/mintosh/services`.

## Runtime interfaces

- MOSF: `/run/mintosh/mosf.sock`; supports `PING`, `VERSION` and `STATUS`.
- launch7762: `/run/mintosh/launch.sock`; supports `LIST`, `START NAME`, `STOP NAME` and `RESTART NAME`.
- Service definitions: `/etc/mintosh/services/*.service`; fields are `ExecStart`, `Enabled` and `Restart`. Restart policies are `no`, `on-failure` and `always`.

systemd remains PID 1 in this release and supervises launch7762. launch7762 supervises Mintosh-managed services. Linux module loading is delegated to kmod; this source does not claim to implement kernel drivers.

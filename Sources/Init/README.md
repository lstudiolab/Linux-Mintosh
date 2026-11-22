# Mintosh Init

Mintosh Init is the first-party PID 1 implementation milestone.

## Implemented

- Refuses to run as an ordinary process; PID 1 is required.
- Mounts devtmpfs, devpts, procfs, sysfs and a volatile /run filesystem.
- Starts the native launch7762 service supervisor directly with execve.
- Reaps adopted child processes.
- Restarts a failed service supervisor using capped exponential backoff.
- Handles SIGTERM and SIGINT and gives the supervisor a bounded graceful-stop window.

## Not production-ready

This binary is not yet selected as the installed system's default init. The system still needs a first-party console/session and login manager, network startup and recovery orchestration, full shutdown ordering, and end-to-end boot tests. systemd remains the current PID 1 until those gates pass. Do not change the boot command line to use Mintosh Init on an installed machine yet.

Build and run its non-privileged checks with:

    make -C Sources/MintoshCore
    Sources/MintoshCore/build/mintosh-init --self-test

The process intentionally exits with status 2 when invoked outside PID 1.

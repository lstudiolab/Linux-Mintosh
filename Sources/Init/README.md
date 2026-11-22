# Mintosh Init

Mintosh Init is the first-party PID 1 implementation milestone.

## Implemented

- Refuses normal execution unless it is PID 1.
- Mounts devtmpfs, devpts, procfs, sysfs and a volatile /run filesystem.
- Starts the native launch7762 service supervisor with execve.
- Reaps adopted child processes while supervising launch7762.
- Restarts a failed supervisor using capped exponential backoff.
- Handles SIGTERM and SIGINT and gives the supervisor a bounded graceful-stop window.
- Provides non-privileged version and self-test commands.

## Not production-ready

This binary is not selected as the installed system's default init. The current image still uses systemd. Mintosh still needs a first-party console/session and login manager, device-event orchestration, network startup, recovery orchestration, full dependency-aware shutdown ordering and end-to-end boot tests. The initramfs currently hands off to the existing system init. Do not change the boot command line to use Mintosh Init on an installed machine yet.

Build and run its non-privileged checks with:

    make -C Sources/MintoshCore
    Sources/MintoshCore/build/mintosh-init --self-test

The process intentionally exits with status 2 when invoked outside PID 1.

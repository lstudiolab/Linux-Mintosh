# LibSys

LibSys provides shared POSIX/Linux primitives for Mintosh native daemons and command-line tools.

## Current API

- Runtime and Unix-socket path selection.
- Recursive runtime-directory creation.
- Complete descriptor writes with short-write and interruption handling.
- Bounded line reads with an absolute monotonic timeout, explicit framing errors and embedded-NUL rejection.
- Unix-domain socket request/response transport.
- syslog integration.

The line reader uses one total deadline rather than resetting the timeout for every byte. This prevents a local client from holding a single-threaded daemon indefinitely by sending a slow trickle of data.

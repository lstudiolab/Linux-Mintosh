# Grand View Dispatch (GVD)

GVD provides two native scheduling interfaces:

- `gvdctl` inspects Linux process state and changes permitted process niceness or scheduling policy through kernel interfaces.
- `libgvd.a` provides a bounded, thread-safe worker queue for Mintosh userspace components.

The worker queue supports non-blocking submission, explicit capacity, idle waiting, graceful drain and worker joining. A full queue returns `EAGAIN`; a shut-down queue returns `ESHUTDOWN`. Task contexts remain caller-owned until callbacks finish. Callbacks must return normally and must not destroy their own queue.

Linux remains responsible for actual CPU scheduling. GVD validates and requests scheduling changes; it does not replace the kernel scheduler.

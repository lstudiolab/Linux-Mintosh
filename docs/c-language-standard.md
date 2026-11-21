# Mintosh native implementation language

Mintosh-owned native userspace is implemented in **ISO C17**.

## Toolchain

- GCC is the baseline compiler in the Debian target environment.
- Clang must remain supported for developer builds.
- The Linux kernel remains upstream and is built with the kernel's own supported toolchain requirements.
- Shell is reserved for boot and image orchestration; Python is used for build validation and release tooling. Neither replaces the native C APIs.

## Source rules

- Compile native components with `-std=c17`; do not rely on compiler-default language modes.
- Use explicit ownership and cleanup for allocated objects and file descriptors.
- Validate lengths, integer overflow, paths, protocol frames and privilege boundaries before acting.
- Prefer bounded buffers and explicit error results over implicit global error state.
- Use direct argv execution; never interpolate service commands into a shell.
- Keep platform-specific Linux interfaces behind small subsystem boundaries.
- Every public API must document ownership, failure behavior and thread/process assumptions.
- Add unit tests for boundary cases and integration tests for IPC, process lifecycle, boot and storage operations.

C17 is the implementation language, not a custom Mintosh scripting language. Application-facing language tooling can be designed later against the stable system APIs.

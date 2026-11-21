# Mintosh Foundation

Foundation is the shared C API layer for native Mintosh userspace. The initial API provides alias-safe strings and byte buffers, generic value arrays, explicit error values, lexical path operations, and bounded regular-file I/O.

## Ownership

- String, data and array values own their allocations and must be destroyed with the matching destroy function.
- Array elements are copied by value; pointers inside an element remain caller-owned.
- File reads reject symlinks, non-regular files and files above the caller-supplied limit.
- Atomic writes create a temporary file in the destination directory, apply the requested mode, write and sync the contents, then rename into place and sync the directory.

## Error handling

Functions returning allocated values use an empty value on failure. File operations accept an optional `MFError` output containing a Foundation error code, the underlying system error and a bounded diagnostic message.

## Build and tests

The native core Makefile builds Foundation into `libfoundation.a`. The test suite covers aliasing, allocation-backed arrays, error lifecycle, bounded reads, symlink rejection, atomic writes, path normalization and cleanup.

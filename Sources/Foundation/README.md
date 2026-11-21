# Foundation

Mintosh Foundation is the small, dependency-light base library shared by native userspace components.

## Current API

- `MFString`: owned NUL-terminated byte strings (UTF-8 by convention; encoding is not validated) with copy, append, equality and destruction operations.
- `MFData`: owned arbitrary byte buffers with copy, append, equality and destruction operations.
- `mf_path_join`, `mf_path_normalize` and `mf_path_is_absolute`: lexical path operations. Normalization does not resolve symbolic links or access the filesystem.

Append operations support source slices that alias the destination buffer, including when allocation moves the buffer. Callers own returned allocations and must destroy them with the matching function.

This is the initial base layer, not a complete Cocoa Foundation replacement. Collections, URL handling, filesystem coordination, serialization and richer error types remain separate implementation work.

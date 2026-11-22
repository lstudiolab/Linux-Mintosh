# Mintosh Package Manager

Mintosh's package system is a first-party operating-system component. The installed OS must not depend on APT, dpkg, RPM tools, Flatpak, AppImage runtimes, or another distribution's package manager.

## Native format

Mintosh's only native installable package format is **`.pkg`**, a Mintosh-defined format. It will have a documented manifest, payload layout, dependency model, permission policy, lifecycle metadata, cryptographic signatures, a native package database, atomic transactions and rollback. It is not Apple's macOS package format and will not reuse proprietary Apple internals.

External Linux package extensions may be recognized for identification or future import planning, but recognition is not installation compatibility. Mintosh must not invoke third-party package managers or silently translate packages while losing dependency, ownership, script, signature or sandbox semantics.

## Current implementation status

`mstore` 0.3.0 is a native CLI shell, not yet a complete package manager:

- `mstore format FILE` identifies known extensions.
- `mstore run FILE.AppImage` can directly execute an explicitly selected AppImage as an unprivileged user. This is an application compatibility path, not a Mintosh system dependency or managed package transaction.
- `mstore install-file FILE` rejects external package formats and reports that the native transaction engine is not ready.
- Search, package inspection, installed-package listing, install, removal, repository update and upgrade report that their native backend is not implemented.

The previous APT/dpkg/Flatpak dispatch has been removed. No external package manager is invoked by mstore. The native `.pkg` installer is deliberately unavailable until signature verification, path and ownership validation, dependency resolution, transaction journaling, rollback and recovery are implemented and tested.

## System integration

The native package manager must own its repository metadata, trust roots, package database, dependency solver, download transport, signature verifier, extraction rules, install/remove lifecycle, permissions and rollback logic. Network access uses kernel-provided sockets and Mintosh-owned networking services. Package installation must never execute package-provided scripts as root without a separately designed, constrained lifecycle mechanism.

## Developer tooling

The Mintosh SDK and IDE remain deferred until the system foundation and the user's programming language are complete. They will build and sign native `.pkg` applications using Mintosh-owned tools.

## Compatibility policy

Compatibility with `.deb`, `.rpm`, `.flatpak` or `.AppImage` is not a dependency requirement. Any future import or launch compatibility must be implemented by Mintosh-owned code, remain optional, and not introduce a third-party package manager or runtime into the installed base system.

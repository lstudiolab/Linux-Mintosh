# Mintosh Package Manager

`mstore` is the native command-line interface for the system package database. Its current implementation uses APT and Debian's signed package repositories as its package transport; Mintosh does not need to operate a package-download server.

## Package format direction

Mintosh's primary native application and distribution package extension will be **`.pkg`**. This is a Mintosh-defined format inspired by the familiar macOS installer-package convention. It will have its own documented specification and implementation; it will not reuse Apple's proprietary package internals or assume that a macOS `.pkg` can be installed on Mintosh.

The future Mintosh Application Manager and package tools are intended to accept supported packages from other Linux ecosystems as well as native `.pkg` packages. Compatibility will be explicit and format-specific:

- **Mintosh `.pkg`**: first-class native format, with a manifest, payload, dependency declarations, permissions, install/remove lifecycle metadata and cryptographic signature.
- **Debian `.deb`**: install through the Debian package database and APT/dpkg compatibility layer where dependencies and system integration are compatible.
- **AppImage**: launch compatible portable Linux applications through a managed integration path; it is not treated as a system package transaction.
- **Flatpak**: support through Flatpak's own runtime, repository and sandbox model when installed and enabled.
- **RPM**: optional import or installation compatibility only where a safe RPM backend and dependency environment are available; no promise of universal cross-distribution compatibility.

The package manager must not silently convert formats in ways that lose dependency, ownership, script, signature or sandbox semantics. It must report unsupported features and reject packages that cannot be installed safely. Native `.pkg` signing, repository trust, transaction rollback and permission policy are release requirements, not optional decoration.

## Implemented local-file handling

The CLI recognizes package extensions without treating the extension alone as proof that a package is safe:

- `mstore format FILE`: identify the file extension and report the corresponding package family.
- `sudo mstore install FILE.deb` or `sudo mstore install-file FILE.deb`: install a local Debian package through APT/dpkg, preserving dependency resolution and the Debian package database.
- `sudo mstore install FILE.flatpak` or `sudo mstore install-file FILE.flatpak`: install a Flatpak bundle through Flatpak's system installation and sandbox model. Flatpak must be installed and configured.
- `mstore run FILE.AppImage`: launch an executable AppImage as the current unprivileged user. The command refuses to launch it as root.
- `.pkg` and `.rpm` are recognized but are not installable yet. The native `.pkg` installer is blocked until its signed format specification, validation and transaction implementation exist. RPM installation remains disabled until dependency and script compatibility can be guaranteed.

Local file inputs are resolved to canonical paths and must be regular files. The manager dispatches only to fixed system executable paths. It does not unpack arbitrary archives itself or silently convert package formats.

## Current commands

- `mstore search QUERY`: search package names.
- `mstore info PACKAGE`: show package metadata.
- `mstore list`: list installed packages.
- `sudo mstore install PACKAGE...`: install repository packages without recommended extras; a supported local file path selects its format-specific backend.
- `sudo mstore remove PACKAGE...`: remove packages.
- `sudo mstore update`: refresh signed repository metadata.
- `sudo mstore upgrade`: upgrade installed packages.

Package arguments are validated before they reach APT. Mutating operations require root privileges; use `sudo` explicitly. The frontend invokes fixed system utility paths rather than resolving package tools through the caller's `PATH`. Package downloads, dependency resolution, signature verification and installation are currently handled by APT/dpkg.

## Developer tooling

A Mintosh SDK and an Xcode-like integrated development environment are planned for a later phase. They are not part of the current unstable system foundation and will not be started now. The SDK/IDE should eventually let developers build, test, sign and publish native `.pkg` applications while also supporting supported Linux application toolchains. Its native-language integration depends on the user's programming language, compiler, runtime and standard library being ready.

## Current scope

The initial catalog remains the configured Debian Trixie repository. The Mintosh-native `.pkg` specification, repository, signing infrastructure, multi-format application manager, graphical catalog, SDK and IDE are future work. The current APT frontend is not yet a complete implementation of those goals.

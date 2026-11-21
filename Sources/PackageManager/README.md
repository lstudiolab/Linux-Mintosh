# Mintosh Package Manager

`mstore` is the native command-line interface for the system package database. The first release uses APT and Debian's signed package repositories as its package transport; Mintosh does not need to operate a package-download server.

## Commands

- `mstore search QUERY`: search package names.
- `mstore info PACKAGE`: show package metadata.
- `mstore list`: list installed packages.
- `sudo mstore install PACKAGE...`: install packages without recommended extras.
- `sudo mstore remove PACKAGE...`: remove packages.
- `sudo mstore update`: refresh signed repository metadata.
- `sudo mstore upgrade`: upgrade installed packages.

Package arguments are validated before they reach APT. Mutating operations require root privileges; use `sudo` explicitly. Package downloads, dependency resolution, signature verification and installation are handled by APT/dpkg.

The initial catalog is the configured Debian Trixie repository. A Mintosh-operated repository and graphical application catalog are separate future work.

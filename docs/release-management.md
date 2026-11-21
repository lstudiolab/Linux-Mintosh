# Mintosh release management

Every Mintosh release is described by `configs/release.json`. Build scripts must use this file rather than independently selecting versions.

## Version fields

- `release`: Mintosh semantic version (for example, `0.1.0`).
- `debian_suite`: Debian suite used to bootstrap the base userspace.
- `architectures`: architectures supported by this release's kernel build.
- `kernel.version`: exact upstream Linux version.
- `kernel.source`: official kernel.org source archive URL.
- `kernel.sha256`: SHA-256 digest checked before extraction.
- `image.primary_architecture`: architecture used for the first bootable installer image.

## Build behavior

The kernel archive is fetched only when building and stored under ignored build directories. The source tree is not committed to Git. Kernel source and object files are kept separate from the release manifest and from other architecture outputs.

Run:

    make check-release
    make release-info
    make kernel-fetch
    make kernel-config
    make kernel
    make test-qemu

The manifest validator is intentionally offline. It verifies the release metadata, URL shape, architecture list and checksum format. The fetch script performs the actual archive checksum verification.

## Updating a release

1. Choose the Mintosh semantic version.
2. Select an exact upstream Linux release.
3. Copy its official kernel.org archive URL and independently verified SHA-256 digest.
4. Update `configs/release.json`.
5. Run `make check-release` and build from a clean checkout.
6. Run the QEMU smoke test and retain its serial log.
7. Publish the ISO and its SHA-256 digest only after the boot test passes.

Never replace an exact kernel version with a moving `latest` URL.

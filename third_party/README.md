# Third-party upstream sources

Lenix tracks large upstream source trees as pinned Git submodules rather than copying their entire histories into the main repository.

- `kernel/linux/` is populated by `make kernel-fetch` from the exact Linux release archive and SHA-256 in `configs/release.json`. It is intentionally not committed because the full kernel tree is very large and should remain a reproducible build input.
- `third_party/freebsd-src/` is a pinned FreeBSD source checkout used only to compare BSD/POSIX behavior. FreeBSD kernel code is not linked into Linux.
- `third_party/xnu/` is a pinned Apple XNU source checkout used only to study Mach task, IPC-port and VM concepts. XNU is not linked into Linux; its per-file APSL-2.0 licensing must be reviewed before any adaptation.

Initialize the reference trees with:

```sh
git submodule update --init --recursive
```

The exact submodule commits are stored as Git gitlinks in this repository. Update them only through reviewed commits that also update `configs/upstream-sources.json` and the license/provenance notes. Do not treat the presence of a reference source tree as permission to copy its code into XCU.

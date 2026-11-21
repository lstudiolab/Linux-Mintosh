# Linux kernel

Mintosh uses the upstream Linux kernel. The source is intentionally fetched during development rather than vendored into Git.

Run `make kernel-fetch` from the repository root. The script reads the current stable release metadata from kernel.org, downloads the matching kernel archive, and extracts it to `kernel/linux/`.

The fetched source directory and build output are ignored by Git. The selected version is recorded in `kernel/LINUX_VERSION`.

Kernel source license: GPL-2.0. See the kernel source tree's `COPYING` file and https://www.kernel.org/doc/html/latest/process/license-rules.html.

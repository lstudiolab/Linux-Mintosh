# Linux kernel

Mintosh uses the upstream Linux kernel. The selected release is pinned in configs/release.json and fetched during development rather than vendored into Git.

Run make kernel-fetch from the repository root. The fetch script downloads the exact manifest version and extracts it to kernel/linux/. The selected version is recorded in kernel/LINUX_VERSION.

The downloaded source and build output are ignored by Git. Do not change the release version by silently tracking the latest stable kernel; update the release manifest as an intentional Mintosh release change.

Kernel source license: GPL-2.0-only. See the kernel source tree's COPYING file and https://www.kernel.org/doc/html/latest/process/license-rules.html.

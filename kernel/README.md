# Legacy Linux preview kernel

The current Developer Preview still uses the upstream Linux kernel as a temporary installer and bring-up environment while the first-party XNV kernel is developed.

Run make kernel-fetch from the repository root. The exact version is pinned in configs/release.json and extracted to kernel/linux/. The downloaded source and build output are ignored by Git.

Linux is not the target kernel for the final Mintosh architecture. Do not add permanent Mintosh subsystems that depend on Linux-specific kernel behavior unless they are explicitly transitional.

Kernel source license: GPL-2.0-only. See the source tree's COPYING file and https://www.kernel.org/doc/html/latest/process/license-rules.html.
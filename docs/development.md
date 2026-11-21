# Development

## Kernel build

```sh
make kernel-fetch
make kernel-config
make kernel
```

Use `ARCH=x86_64` for 64-bit Intel/AMD or `ARCH=arm64` for 64-bit ARM.

## Build output

Kernel build artifacts are written to `out/kernel/`. Downloaded upstream source is stored in `kernel/linux/`. Both locations are excluded from Git.

## Safety

Do not install a newly built kernel onto your daily-use machine. Validate it in a virtual machine first. Keep a known-good boot entry and recovery media when testing on physical hardware.

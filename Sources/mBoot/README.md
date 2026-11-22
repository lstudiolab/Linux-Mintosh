# mBoot

mBoot is Mintosh's native x86-64 UEFI boot application. The firmware loads the fallback image
`EFI/BOOT/BOOTX64.EFI` (built from `boot.efi`); installed systems can also
register `EFI/Mintosh/boot.efi`. mBoot locates the boot volume, validates that the
kernel and initramfs are present and readable, and asks UEFI to load and start
the Linux kernel's EFI stub.

## Current scope

- x86-64 UEFI application built with GNU-EFI.
- Loads the EFI-stub Linux kernel from `\\boot\\vmlinuz`.
- Supplies `initrd=\\boot\\initramfs.cpio.gz` through the EFI load options.
- Uses firmware `LoadImage` policy, so firmware Secure Boot policy applies
  when the platform enforces image authentication.
- Fails closed when the boot volume, kernel, or initramfs is unavailable.

## Security and limitations

mBoot does not bypass Secure Boot. The firmware must trust and authenticate
mBoot, and firmware image-loading policy must authenticate the kernel. Mintosh
does not yet ship its own signing key or signed release chain.

TPM measured boot is not implemented in this initial version. Firmware may
measure its own boot stages; a later mBoot phase will use the TCG2 protocol to
extend measurements for the kernel and initramfs. A TPM stores measurements;
it does not execute a second bootloader.

This is an initial bootloader implementation, not yet the default ISO boot
path. The existing GRUB path remains available until mBoot has passed OVMF
Secure Boot, normal QEMU boot, and disposable-disk installer integration tests.

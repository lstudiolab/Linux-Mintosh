#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
ARCH=${ARCH:-x86_64}
[ "$ARCH" = x86_64 ] || { echo "The live ISO currently targets x86_64." >&2; exit 1; }
ISO_ROOT="$ROOT/out/iso"
ISO="$ROOT/out/mintosh-$ARCH.iso"
ROOTFS="$ROOT/out/rootfs"
[ "$(id -u)" -eq 0 ] || { echo "Run through sudo: make iso" >&2; exit 1; }
[ -s "$ROOT/out/initramfs.cpio.gz" ] || { echo "Run make initramfs first." >&2; exit 1; }
rm -rf "$ISO_ROOT"
mkdir -p "$ISO_ROOT/boot/grub"
KERNEL_IMAGE=$(find "$ROOTFS/boot" -maxdepth 1 -type f -name 'vmlinuz-*' -print -quit)
[ -n "$KERNEL_IMAGE" ] || { echo "Kernel image missing from rootfs." >&2; exit 1; }
install -m 0644 "$KERNEL_IMAGE" "$ISO_ROOT/boot/vmlinuz"
install -m 0644 "$ROOT/out/initramfs.cpio.gz" "$ISO_ROOT/boot/initramfs.cpio.gz"
cat > "$ISO_ROOT/boot/grub/grub.cfg" <<'EOF'
set timeout=5
set default=0
menuentry "Mintosh Live Installer" {
    linux /boot/vmlinuz console=tty0 console=ttyS0,115200n8 loglevel=4 rdinit=/init
    initrd /boot/initramfs.cpio.gz
}
menuentry "Mintosh Live Installer (safe graphics/verbose)" {
    linux /boot/vmlinuz console=tty0 console=ttyS0,115200n8 loglevel=7 nomodeset rdinit=/init
    initrd /boot/initramfs.cpio.gz
}
EOF
grub-mkrescue -o "$ISO" -volid MINTOSH "$ISO_ROOT"
ISO_LABEL=$(blkid -p -s LABEL -o value "$ISO" 2>/dev/null || true)
[ "$ISO_LABEL" = "MINTOSH" ] || { echo "ISO volume label verification failed; the installer cannot safely identify its boot medium." >&2; exit 1; }
chmod 0644 "$ISO"
echo "Bootable Mintosh installer ISO created: out/mintosh-$ARCH.iso"

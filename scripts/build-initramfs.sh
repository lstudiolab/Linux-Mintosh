#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
ROOTFS="$ROOT/out/rootfs"
OUT="$ROOT/out/initramfs.cpio.gz"
[ "$(id -u)" -eq 0 ] || { echo "Run through sudo: make initramfs" >&2; exit 1; }
[ -x "$ROOTFS/sbin/init" ] || { echo "Root filesystem is missing systemd. Run make rootfs." >&2; exit 1; }
KERNEL_IMAGE=$(find "$ROOTFS/boot" -maxdepth 1 -type f -name 'vmlinuz-*' -print -quit)
[ -n "$KERNEL_IMAGE" ] || { echo "Versioned kernel image missing. Run make kernel-install first." >&2; exit 1; }
install -m 0755 "$ROOT/scripts/mintosh-installer.sh" "$ROOTFS/usr/local/sbin/mintosh-installer"
install -m 0644 "$ROOT/scripts/installer-disk-safety.sh" "$ROOTFS/usr/local/sbin/installer-disk-safety.sh"
install -m 0644 "$ROOT/scripts/installer-storage.sh" "$ROOTFS/usr/local/sbin/installer-storage.sh"
mkdir -p "$ROOT/out"
"${CC:-cc}" -static -Os -std=c17 -Wall -Wextra -Werror \
  "$ROOT/Sources/Boot/early_init.c" -o "$ROOT/out/mintosh-init"
install -m 0755 "$ROOT/out/mintosh-init" "$ROOTFS/init"
echo "Packing complete Mintosh live environment into initramfs..."
(cd "$ROOTFS" && find . -xdev ! -path "./boot/vmlinuz-*" -print0 | cpio --null --create --format=newc --quiet | gzip -1) > "$OUT"
chmod 0644 "$OUT"
gzip -t "$OUT"
echo "Initramfs created: out/initramfs.cpio.gz ($(du -h "$OUT" | awk '{print $1}'))"

#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
ROOTFS="$ROOT/out/rootfs"
STAGING="$ROOT/out/bootstrap-rootfs"
OUT="$ROOT/out/initramfs.cpio.gz"
[ "$(id -u)" -eq 0 ] || { echo "Run through sudo: make initramfs" >&2; exit 1; }
[ -x "$ROOTFS/sbin/init" ] || { echo "Root filesystem is missing systemd. Run make rootfs." >&2; exit 1; }
KERNEL_IMAGE=$(find "$ROOTFS/boot" -maxdepth 1 -type f -name 'vmlinuz-*' -print -quit)
[ -n "$KERNEL_IMAGE" ] || { echo "Versioned kernel image missing. Run make kernel-install first." >&2; exit 1; }

SOURCE_COMMIT=$(git -c safe.directory="$ROOT" -C "$ROOT" rev-parse HEAD 2>/dev/null || true)
[ "$(printf '%s' "$SOURCE_COMMIT" | wc -c)" -eq 40 ] &&
  printf '%s' "$SOURCE_COMMIT" | grep -Eq '^[0-9a-f]+$' || {
    echo "Build the installer from a Git checkout with a full commit ID." >&2
    exit 1
  }
git -c safe.directory="$ROOT" -C "$ROOT" diff --quiet && git -c safe.directory="$ROOT" -C "$ROOT" diff --cached --quiet || {
  echo "Commit source changes before building the installer; the ISO pins a Git commit." >&2
  exit 1
}

rm -rf "$STAGING"
cp -a "$ROOTFS" "$STAGING"
# Build tools live only in this temporary copy, not in the installed rootfs.
rm -f "$STAGING/etc/resolv.conf"
cp /etc/resolv.conf "$STAGING/etc/resolv.conf"
chroot "$STAGING" apt-get update
chroot "$STAGING" apt-get install --yes --no-install-recommends \
  git debootstrap build-essential bc bison flex libelf-dev libncurses-dev \
  libssl-dev libsqlite3-dev gnu-efi qemu-system-x86 xorriso mtools cpio \
  parted python3 curl ca-certificates zip xz-utils rsync sudo dialog \
  efibootmgr grub-common grub-pc-bin grub-efi-amd64-bin dosfstools \
  util-linux kmod pciutils usbutils acpi iw ethtool network-manager \
  wpasupplicant
chroot "$STAGING" apt-get clean
rm -f "$STAGING/etc/resolv.conf"
ln -snf /run/systemd/resolve/stub-resolv.conf "$STAGING/etc/resolv.conf"
rm -rf "$STAGING/var/lib/apt/lists/"*

install -m 0755 "$ROOT/scripts/mintosh-installer.sh" "$STAGING/usr/local/sbin/mintosh-installer"
install -m 0755 "$ROOT/scripts/installer-bootstrap.sh" "$STAGING/usr/local/sbin/installer-bootstrap.sh"
install -m 0644 "$ROOT/scripts/installer-disk-safety.sh" "$STAGING/usr/local/sbin/installer-disk-safety.sh"
install -m 0644 "$ROOT/scripts/installer-storage.sh" "$STAGING/usr/local/sbin/installer-storage.sh"
mkdir -p "$STAGING/etc/mintosh"
printf "MINTOSH_SOURCE_REPOSITORY='https://github.com/lstudiolab/Linux-Mintosh.git'\nMINTOSH_SOURCE_COMMIT='%s'\n" \
  "$SOURCE_COMMIT" > "$STAGING/etc/mintosh/installer-source.env"
chmod 0644 "$STAGING/etc/mintosh/installer-source.env"

COMPILER=cc
"$COMPILER" -static -Os -std=c17 -Wall -Wextra -Werror \
  "$ROOT/Sources/Boot/early_init.c" -o "$ROOT/out/mintosh-init"
install -m 0755 "$ROOT/out/mintosh-init" "$STAGING/init"
"$COMPILER" -static -O2 -std=c17 -Wall -Wextra -Werror \
  "$ROOT/Sources/Installer/hardware_audit.c" -o "$STAGING/usr/local/sbin/mintosh-hardware-audit"
"$STAGING/usr/local/sbin/mintosh-hardware-audit" --self-test >/dev/null

mkdir -p "$ROOT/out"
echo "Packing the source-build Mintosh bootstrap environment..."
(cd "$STAGING" && find . -xdev ! -path "./boot/vmlinuz-*" -print0 | cpio --null --create --format=newc --quiet | gzip -1) > "$OUT"
chmod 0644 "$OUT"
gzip -t "$OUT"
echo "Bootstrap initramfs created: out/initramfs.cpio.gz ($(du -h "$OUT" | awk '{print $1}'))"
echo "Pinned source commit: $SOURCE_COMMIT"

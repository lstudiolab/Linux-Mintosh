#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
ROOTFS="$ROOT/out/rootfs"
OUT="$ROOT/out/initramfs.cpio.gz"
[ "$(id -u)" -eq 0 ] || { echo "Run through sudo: make initramfs" >&2; exit 1; }
[ -x "$ROOTFS/sbin/init" ] || { echo "Root filesystem is missing systemd. Run make rootfs." >&2; exit 1; }
KERNEL_IMAGE=$(find "$ROOTFS/boot" -maxdepth 1 -type f -name 'vmlinuz-*' -print -quit)\n[ -n "$KERNEL_IMAGE" ] || { echo "Versioned kernel image missing. Run make kernel-install first." >&2; exit 1; }
install -m 0755 "$ROOT/scripts/mintosh-installer.sh" "$ROOTFS/usr/local/sbin/mintosh-installer"
cat > "$ROOTFS/init" <<'EOF'
#!/bin/sh
mount -t devtmpfs devtmpfs /dev 2>/dev/null || true
mkdir -p /dev/pts /proc /sys /run
mount -t devpts devpts /dev/pts 2>/dev/null || true
mount -t proc proc /proc
mount -t sysfs sysfs /sys
mount -t tmpfs -o mode=0755,nosuid,nodev tmpfs /run
mkdir -p /run/lock
if [ -x /usr/bin/systemd-machine-id-setup ]; then
  /usr/bin/systemd-machine-id-setup --commit 2>/dev/null || true
fi
exec /sbin/init
EOF
chmod 0755 "$ROOTFS/init"
mkdir -p "$ROOT/out"
echo "Packing complete Mintosh live environment into initramfs..."
(cd "$ROOTFS" && find . -xdev ! -path "./boot/vmlinuz-*" -print0 | cpio --null --create --format=newc --quiet | gzip -1) > "$OUT"
chmod 0644 "$OUT"
echo "Initramfs created: out/initramfs.cpio.gz ($(du -h "$OUT" | awk '{print $1}'))"

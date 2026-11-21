#!/bin/sh
set -eu
ISO=${1:-}
DEVICE=${2:-}
[ "$(id -u)" -eq 0 ] || { echo "Run with sudo." >&2; exit 1; }
[ -s "$ISO" ] || { echo "ISO file not found: $ISO" >&2; exit 1; }
[ -b "$DEVICE" ] || { echo "Target must be a whole block device, e.g. /dev/sdX." >&2; exit 1; }
[ "$(lsblk -dn -o TYPE "$DEVICE")" = disk ] || { echo "Refusing to write to a partition. Select the whole USB drive." >&2; exit 1; }
echo "Target device:"
lsblk -o NAME,SIZE,MODEL,TYPE,MOUNTPOINTS "$DEVICE"
echo
echo "WARNING: every partition and file on $DEVICE will be erased."
printf 'Type WRITE %s to continue: ' "$DEVICE"
read -r CONFIRM
[ "$CONFIRM" = "WRITE $DEVICE" ] || { echo "Confirmation did not match; nothing written."; exit 1; }
lsblk -lnpo NAME "$DEVICE" | tail -n +2 | while IFS= read -r part; do
  umount "$part" 2>/dev/null || true
done
echo "Writing Mintosh installer image..."
dd if="$ISO" of="$DEVICE" bs=4M status=progress conv=fsync
sync
echo "USB installer is ready. Safely eject it before removing it."

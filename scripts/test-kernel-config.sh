#!/bin/sh
set -eu
CONFIG=$(mktemp)
trap 'rm -f "$CONFIG"' EXIT HUP INT TERM
for option in \
  CONFIG_MODULES CONFIG_MODULE_SIG CONFIG_MODULE_SIG_ALL CONFIG_MODULE_SIG_FORCE \
  CONFIG_MODULE_SIG_SHA256 CONFIG_BLK_DEV_INITRD CONFIG_DEVTMPFS CONFIG_DEVTMPFS_MOUNT \
  CONFIG_EXT4_FS CONFIG_VIRTIO CONFIG_VIRTIO_BLK CONFIG_VIRTIO_NET \
  CONFIG_SERIAL_8250 CONFIG_SERIAL_8250_CONSOLE CONFIG_PROC_FS CONFIG_SYSFS \
  CONFIG_TMPFS CONFIG_CGROUPS CONFIG_SECCOMP CONFIG_SECCOMP_FILTER
do
  printf '%s=y\n' "$option" >> "$CONFIG"
done
sh scripts/verify-kernel-config.sh "$CONFIG" >/dev/null
sed -i '/^CONFIG_MODULE_SIG_FORCE=y$/d' "$CONFIG"
if sh scripts/verify-kernel-config.sh "$CONFIG" >/dev/null 2>&1; then
  echo "Kernel config verifier accepted a missing required option." >&2
  exit 1
fi
echo "Kernel configuration verification tests passed."

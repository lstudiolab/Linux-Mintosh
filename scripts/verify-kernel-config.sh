#!/bin/sh
set -eu
[ "$#" -eq 1 ] || { echo "Usage: verify-kernel-config.sh KERNEL_CONFIG" >&2; exit 64; }
CONFIG=$1
[ -f "$CONFIG" ] || { echo "Kernel configuration not found: $CONFIG" >&2; exit 1; }
required='
CONFIG_MODULES
CONFIG_MODULE_SIG
CONFIG_MODULE_SIG_ALL
CONFIG_MODULE_SIG_FORCE
CONFIG_MODULE_SIG_SHA256
CONFIG_BLK_DEV_INITRD
CONFIG_DEVTMPFS
CONFIG_DEVTMPFS_MOUNT
CONFIG_EXT4_FS
CONFIG_VIRTIO
CONFIG_VIRTIO_BLK
CONFIG_VIRTIO_NET
CONFIG_SERIAL_8250
CONFIG_SERIAL_8250_CONSOLE
CONFIG_PROC_FS
CONFIG_SYSFS
CONFIG_TMPFS
CONFIG_CGROUPS
CONFIG_SECCOMP
CONFIG_SECCOMP_FILTER
'
for option in $required; do
    if ! grep -qx "${option}=y" "$CONFIG"; then
        echo "Required kernel option is not enabled: ${option}" >&2
        exit 1
    fi
done
echo "Kernel configuration includes the required boot, virtualization and module-signing options."

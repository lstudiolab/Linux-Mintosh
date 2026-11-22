#!/bin/sh
set -eu
[ "$#" -ge 1 ] && [ "$#" -le 3 ] || { echo "Usage: verify-kernel-config.sh KERNEL_CONFIG [PROFILE] [x86_64|arm64]" >&2; exit 64; }
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
ARCH=${3:-x86_64}
case "$ARCH" in
    x86_64)
        for option in CONFIG_IWLWIFI CONFIG_IWLMVM CONFIG_R8169 CONFIG_DRM_I915 CONFIG_NVME_CORE CONFIG_BLK_DEV_NVME CONFIG_SATA_AHCI; do
            if ! grep -Eq "^${option}=(m|y)$" "$CONFIG"; then
                echo "Generic x86-64 hardware driver is not enabled: ${option}" >&2
                exit 1
            fi
        done
        ;;
    arm64) ;;
    *) echo "Unsupported kernel architecture: $ARCH" >&2; exit 64 ;;
esac
PROFILE=${2:-generic}
case "$PROFILE" in
    generic) ;;
    hp-probook|hp-probook-450-g6)
        [ "$ARCH" = x86_64 ] || { echo "HP ProBook profile requires x86_64." >&2; exit 1; }
        for option in CONFIG_HP_WMI CONFIG_HP_ACCEL CONFIG_RTW88_8821CE CONFIG_RTW88_8822BE; do
            if ! grep -Eq "^${option}=(m|y)$" "$CONFIG"; then
                echo "HP ProBook profile driver is not enabled: ${option}" >&2
                exit 1
            fi
        done
        ;;
    *) echo "Unknown hardware profile: $PROFILE" >&2; exit 64 ;;
esac
echo "Kernel configuration includes required boot, generic hardware and $PROFILE profile options."

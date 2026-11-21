#!/usr/bin/env bash
set -Eeuo pipefail
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
mkdir -p "$TMP/bin"
cat > "$TMP/bin/blkid" <<'SH'
#!/bin/sh
if [ "${MINTOSH_TEST_NO_LABEL:-0}" = 1 ]; then exit 2; fi
[ "$1" = "-c" ] && [ "$2" = "/dev/null" ] && [ "$3" = "-t" ] && [ "$4" = "LABEL=MINTOSH" ] && [ "$5" = "-o" ] && [ "$6" = "device" ] || exit 64
printf '%s\n' /dev/sdb1
SH
cat > "$TMP/bin/lsblk" <<'SH'
#!/bin/sh
if [ "$1" = "-nro" ] && [ "$2" = "PKNAME" ]; then
    [ "$3" = "/dev/sdb1" ] && printf '%s\n' sdb
    exit 0
fi
if [ "$1" = "-nro" ] && [ "$2" = "NAME" ]; then
    [ "$3" = "/dev/sdb1" ] && printf '%s\n' sdb1
    exit 0
fi
if [ "$1" = "-J" ]; then
    cat <<'JSON'
{"blockdevices":[
 {"name":"sda","size":"500G","model":"Internal SSD","type":"disk","rm":false},
 {"name":"sdb","size":"32G","model":"Mintosh Installer USB","type":"disk","rm":false},
 {"name":"sde","size":"16G","model":"Second Mintosh boot medium","type":"disk","rm":false},
 {"name":"sdc","size":"64G","model":"Backup USB","type":"disk","rm":true},
 {"name":"sr0","size":"700M","model":"Optical drive","type":"rom","rm":true}
]}
JSON
    exit 0
fi
exit 64
SH
chmod 0755 "$TMP/bin/blkid" "$TMP/bin/lsblk"
PATH="$TMP/bin:$PATH"
source "$ROOT/scripts/installer-disk-safety.sh"
mintosh_discover_protected_disks
[[ "${#MINTOSH_PROTECTED_DISKS[@]}" -eq 2 ]]
mintosh_disk_is_protected /dev/sdb
mintosh_disk_is_protected /dev/sde
! mintosh_disk_is_protected /dev/sda
DISKS=$(mintosh_list_installable_disks)
[[ "$DISKS" == "/dev/sda 500G Internal SSD" ]]
MINTOSH_TEST_NO_LABEL=1
export MINTOSH_TEST_NO_LABEL
if mintosh_discover_protected_disks; then
    echo "Installer accepted a boot environment without a discoverable boot medium." >&2
    exit 1
fi
echo "Installer boot-media exclusion tests passed."

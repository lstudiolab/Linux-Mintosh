#!/usr/bin/env bash
set -Eeuo pipefail
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
source "$ROOT/scripts/installer-storage.sh"
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
IMAGE="$TMP/disk.img"
truncate -s 50G "$IMAGE"
parted -s "$IMAGE" mklabel gpt
parted -s "$IMAGE" mkpart primary ext4 1MiB 8GiB
parted -s "$IMAGE" mkpart primary ext4 9GiB 16GiB
SENTINEL="MINTOSH-KEEP-THIS"
printf '%s' "$SENTINEL" | dd of="$IMAGE" bs=1 seek=2097152 conv=notrunc status=none
BEFORE=$(python3 - "$IMAGE" <<'PY'
import json,subprocess,sys
out=subprocess.check_output(["parted","-m","-s",sys.argv[1],"unit","MiB","print"],text=True)
parts=[]
for line in out.splitlines():
    p=line.rstrip(";").split(":")
    if len(p)>=4 and p[0].isdigit() and int(p[0])<=2:
        parts.append((p[0],p[1],p[2]))
print(json.dumps(parts))
PY
)
FREE=$(mintosh_find_largest_free_region "$IMAGE" 20480)
read -r START END <<<"$FREE"
parted -s "$IMAGE" mkpart primary ext4 "$START" "$END"
AFTER=$(python3 - "$IMAGE" <<'PY'
import json,subprocess,sys
out=subprocess.check_output(["parted","-m","-s",sys.argv[1],"unit","MiB","print"],text=True)
parts=[]
for line in out.splitlines():
    p=line.rstrip(";").split(":")
    if len(p)>=4 and p[0].isdigit() and int(p[0])<=2:
        parts.append((p[0],p[1],p[2]))
print(json.dumps(parts))
PY
)
[[ "$BEFORE" == "$AFTER" ]]
ACTUAL=$(dd if="$IMAGE" bs=1 skip=2097152 count="${#SENTINEL}" status=none)
[[ "$ACTUAL" == "$SENTINEL" ]]
PARTITION_COUNT=$(parted -m -s "$IMAGE" unit MiB print | awk -F: '$1 ~ /^[0-9]+$/ { count++ } END { print count+0 }')
[[ "$PARTITION_COUNT" -eq 3 ]]
if mintosh_find_largest_free_region "$IMAGE" 50000 >/dev/null 2>&1; then
    echo "Installer accepted insufficient free space." >&2
    exit 1
fi
echo "Installer alongside partition preservation tests passed."

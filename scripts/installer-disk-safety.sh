#!/usr/bin/env bash
# Disk discovery shared by the live installer and its safety tests.
MINTOSH_LIVE_LABEL=MINTOSH
MINTOSH_PROTECTED_DISKS=()

mintosh_discover_protected_disks() {
    local source parent name disk
    local -a sources=()
    mapfile -t sources < <(blkid -c /dev/null -t "LABEL=$MINTOSH_LIVE_LABEL" -o device 2>/dev/null | sort -u)
    (("${#sources[@]}" > 0)) || return 1

    MINTOSH_PROTECTED_DISKS=()
    for source in "${sources[@]}"; do
        [[ "$source" == /dev/* ]] || continue
        parent=$(lsblk -ndo PKNAME "$source" 2>/dev/null | head -n1 || true)
        if [[ -n "$parent" ]]; then
            disk="/dev/$parent"
        else
            name=$(lsblk -ndo NAME "$source" 2>/dev/null | head -n1 || true)
            [[ -n "$name" ]] || continue
            disk="/dev/$name"
        fi
        MINTOSH_PROTECTED_DISKS+=("$disk")
    done
    (("${#MINTOSH_PROTECTED_DISKS[@]}" > 0))
}

mintosh_disk_is_protected() {
    local candidate=$1 protected
    for protected in "${MINTOSH_PROTECTED_DISKS[@]}"; do
        [[ "$candidate" == "$protected" ]] && return 0
    done
    return 1
}

mintosh_list_installable_disks() {
    (("${#MINTOSH_PROTECTED_DISKS[@]}" > 0)) || return 1
    python3 - "${MINTOSH_PROTECTED_DISKS[@]}" <<'PY'
import json
import subprocess
import sys

protected = set(sys.argv[1:])
try:
    raw = subprocess.check_output(
        ["lsblk", "-J", "-d", "-o", "NAME,SIZE,MODEL,TYPE,RM"],
        text=True
    )
    data = json.loads(raw)
except Exception as error:
    print(f"installer: cannot read block-device inventory: {error}", file=sys.stderr)
    raise SystemExit(1)

count = 0
for device in data.get("blockdevices", []):
    if device.get("type") != "disk":
        continue
    removable = str(device.get("rm", "")).lower() in {"1", "true", "yes"}
    if removable:
        continue
    path = "/dev/" + str(device.get("name", ""))
    if path in protected:
        continue
    size = str(device.get("size", "unknown")).replace("\\t", " ").replace("\\n", " ")
    model = str(device.get("model") or "Unidentified disk").replace("\\t", " ").replace("\\n", " ")
    print(f"{path} {size} {model}")
    count += 1

if count == 0:
    raise SystemExit(3)
PY
}

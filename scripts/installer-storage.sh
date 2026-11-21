#!/usr/bin/env bash
mintosh_find_largest_free_region() {
    local disk=$1 minimum_mib=${2:-20480}
    python3 - "$disk" "$minimum_mib" <<'PY'
import subprocess
import sys

disk = sys.argv[1]
minimum = float(sys.argv[2])
try:
    output = subprocess.check_output(
        ["parted", "-m", "-s", disk, "unit", "MiB", "print", "free"],
        text=True, stderr=subprocess.DEVNULL
    )
except (OSError, subprocess.CalledProcessError) as error:
    print(f"installer: cannot inspect free space on {disk}: {error}", file=sys.stderr)
    raise SystemExit(1)

best = None
for line in output.splitlines():
    fields = line.rstrip(";").split(":")
    if len(fields) < 4 or fields[0] != "":
        continue
    try:
        start = float(fields[1].removesuffix("MiB"))
        end = float(fields[2].removesuffix("MiB"))
        size = float(fields[3].removesuffix("MiB"))
    except ValueError:
        continue
    if best is None or size > best[0]:
        best = (size, start, end)

if best is None or best[0] < minimum:
    raise SystemExit(1)
print(f"{best[1]:.2f}MiB {best[2]:.2f}MiB")
PY
}

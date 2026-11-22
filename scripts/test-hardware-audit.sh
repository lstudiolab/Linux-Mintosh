#!/usr/bin/env sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
OUT="$ROOT/out/mintosh-hardware-audit-test"
mkdir -p "$ROOT/out"
"${CC:-cc}" -std=c17 -O2 -Wall -Wextra -Werror "$ROOT/Sources/Installer/hardware_audit.c" -o "$OUT"
"$OUT" --self-test >/dev/null
COUNT=$("$OUT" --list | wc -l | tr -d ' ')
[ "$COUNT" = 150 ] || { echo "Expected 150 checks, got $COUNT" >&2; exit 1; }
echo "Hardware audit registry: 150 unique checks."

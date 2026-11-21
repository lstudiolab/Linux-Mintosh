#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
ISO=${1:-"$ROOT/out/mintosh-x86_64.iso"}
LOG="$ROOT/out/qemu-boot.log"
QEMU=${QEMU:-qemu-system-x86_64}
TIMEOUT=${QEMU_TIMEOUT:-120}
command -v "$QEMU" >/dev/null || { echo "Missing $QEMU. Install qemu-system-x86." >&2; exit 1; }
[ -s "$ISO" ] || { echo "ISO not found: $ISO" >&2; exit 1; }
echo "Booting $ISO in QEMU (up to $TIMEOUT seconds)..."
set +e
timeout --signal=TERM "$TIMEOUT" "$QEMU" -m 2048 -smp 2 \
  -netdev user,id=net0 -device virtio-net-pci,netdev=net0 \
  -cdrom "$ISO" -boot d -display none -serial stdio -monitor none -no-reboot > "$LOG" 2>&1
STATUS=$?
set -e
if grep -q "Mintosh Base is ready" "$LOG"; then
  echo "PASS: Mintosh reached its multi-user boot target."
  echo "Serial log: out/qemu-boot.log"
  exit 0
fi
echo "FAIL: Mintosh did not reach the ready marker (QEMU exit: $STATUS)." >&2
tail -n 80 "$LOG" >&2
exit 1

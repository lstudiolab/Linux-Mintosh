#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
ISO=${1:-"$ROOT/out/mintosh-x86_64.iso"}
QEMU=${QEMU:-qemu-system-x86_64}
TIMEOUT=${QEMU_TIMEOUT:-180}
MEMORY=${QEMU_MEMORY:-4096}
command -v "$QEMU" >/dev/null || { echo "Missing $QEMU. Install qemu-system-x86." >&2; exit 1; }
[ -s "$ISO" ] || { echo "ISO not found: $ISO" >&2; exit 1; }
mkdir -p "$ROOT/out"

run_boot_test() {
  MODE=$1
  LOG="$ROOT/out/qemu-boot-$MODE.log"
  shift
  echo "Booting $ISO in QEMU ($MODE firmware, $MEMORY MiB RAM, timeout $TIMEOUT seconds)..."
  set +e
  timeout --signal=TERM "$TIMEOUT" "$QEMU" -m "$MEMORY" -smp 2 "$@" \
    -netdev user,id=net0 -device virtio-net-pci,netdev=net0 \
    -cdrom "$ISO" -boot d -display none -serial stdio -monitor none -no-reboot > "$LOG" 2>&1
  STATUS=$?
  set -e
  if grep -q "Mintosh Base is ready" "$LOG"; then
    echo "PASS: Mintosh reached its multi-user boot target using $MODE firmware."
    return 0
  fi
  echo "FAIL: Mintosh did not reach the ready marker using $MODE firmware (QEMU exit: $STATUS)." >&2
  tail -n 80 "$LOG" >&2
  return 1
}

run_boot_test bios

OVMF_CODE=
OVMF_VARS=
if [ -f /usr/share/OVMF/OVMF_CODE_4M.fd ] && [ -f /usr/share/OVMF/OVMF_VARS_4M.fd ]; then
  OVMF_CODE=/usr/share/OVMF/OVMF_CODE_4M.fd
  OVMF_VARS=/usr/share/OVMF/OVMF_VARS_4M.fd
elif [ -f /usr/share/OVMF/OVMF_CODE.fd ] && [ -f /usr/share/OVMF/OVMF_VARS.fd ]; then
  OVMF_CODE=/usr/share/OVMF/OVMF_CODE.fd
  OVMF_VARS=/usr/share/OVMF/OVMF_VARS.fd
elif [ -f /usr/share/qemu/OVMF_CODE.fd ] && [ -f /usr/share/qemu/OVMF_VARS.fd ]; then
  OVMF_CODE=/usr/share/qemu/OVMF_CODE.fd
  OVMF_VARS=/usr/share/qemu/OVMF_VARS.fd
fi
[ -n "$OVMF_CODE" ] || { echo "OVMF firmware files not found. Install the ovmf package; UEFI boot verification is required." >&2; exit 1; }
VARS_COPY="$ROOT/out/OVMF_VARS.test.fd"
cp "$OVMF_VARS" "$VARS_COPY"
trap 'rm -f "$VARS_COPY"' EXIT HUP INT TERM
run_boot_test uefi -machine q35 \
  -drive "if=pflash,format=raw,unit=0,file=$OVMF_CODE,readonly=on" \
  -drive "if=pflash,format=raw,unit=1,file=$VARS_COPY"

cp "$ROOT/out/qemu-boot-uefi.log" "$ROOT/out/qemu-boot.log"
echo "PASS: BIOS and UEFI live boot tests completed."
echo "Serial logs: out/qemu-boot-bios.log and out/qemu-boot-uefi.log"

#!/bin/sh
set -eu
IMAGE="${1:-out/xnv/XNV.EFI}"
OVMF_CODE="${OVMF_CODE:-}"
if [ -z "$OVMF_CODE" ]; then
  for candidate in /usr/share/OVMF/OVMF_CODE.fd /usr/share/OVMF/OVMF_CODE_4M.fd /usr/share/edk2/ovmf/OVMF_CODE.fd /usr/share/qemu/OVMF.fd; do
    if [ -s "$candidate" ]; then OVMF_CODE="$candidate"; break; fi
  done
fi
if [ -z "$OVMF_CODE" ]; then
  OVMF_CODE=$(find /usr/share -type f \\( -iname "OVMF_CODE*.fd" -o -iname "OVMF.fd" \\) -print -quit 2>/dev/null || true)
fi
ESP_DIR="out/xnv-esp"
LOG="out/xnv-qemu.log"
command -v qemu-system-x86_64 >/dev/null 2>&1 || { echo "qemu-system-x86_64 is required" >&2; exit 1; }
command -v timeout >/dev/null 2>&1 || { echo "timeout is required" >&2; exit 1; }
test -s "$IMAGE" || { echo "XNV EFI image is missing: $IMAGE" >&2; exit 1; }
test -s "$OVMF_CODE" || { echo "OVMF firmware is missing: $OVMF_CODE" >&2; exit 1; }
rm -rf "$ESP_DIR"
mkdir -p "$ESP_DIR/EFI/BOOT"
cp "$IMAGE" "$ESP_DIR/EFI/BOOT/BOOTX64.EFI"
rm -f "$LOG"
set +e
timeout 25s qemu-system-x86_64 -machine q35 -cpu qemu64 -m 2048 \
  -drive "format=raw,file=fat:rw:$ESP_DIR" -bios "$OVMF_CODE" \
  -serial "file:$LOG" -monitor none -display none -net none -no-reboot -no-shutdown
status=$?
set -e
if [ "$status" -ne 124 ]; then
  echo "QEMU did not remain in the expected halted-kernel state (exit $status)." >&2
  test -f "$LOG" && cat "$LOG" >&2
  exit 1
fi
for expected in \
  "XNV: UEFI boot ABI v1 accepted" \
  "XNV: GDT and IDT installed" \
  "XNV: physical page allocator ready" \
  "XNV: paging enabled; boot services exited" \
  "XNV: bootstrap reached idle state"; do
  grep -Fq "$expected" "$LOG" || { echo "Missing XNV boot milestone: $expected" >&2; cat "$LOG" >&2; exit 1; }
done
echo "XNV UEFI boot test passed."
cat "$LOG"

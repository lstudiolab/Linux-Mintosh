#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
EFI_APP=${1:?usage: test-mboot-qemu.sh BOOTX64.EFI KERNEL INITRAMFS}
KERNEL=${2:?missing kernel image}
INITRAMFS=${3:?missing initramfs}
QEMU=${QEMU:-qemu-system-x86_64}
TIMEOUT=${QEMU_TIMEOUT:-180}
MEMORY=${QEMU_MEMORY:-4096}
ESP="$ROOT/out/mboot-test-esp.img"
VARS_COPY="$ROOT/out/OVMF_MBOOT_VARS.test.fd"
LOG="$ROOT/out/qemu-mboot-uefi.log"

for file in "$EFI_APP" "$KERNEL" "$INITRAMFS"; do
  [ -s "$file" ] || { echo "Required mBoot test input is missing: $file" >&2; exit 1; }
done
for command in "$QEMU" truncate mformat mmd mcopy timeout; do
  command -v "$command" >/dev/null || {
    echo "Missing mBoot QEMU test dependency: $command" >&2
    exit 1
  }
done

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
[ -n "$OVMF_CODE" ] || { echo "OVMF firmware files not found. Install ovmf." >&2; exit 1; }

mkdir -p "$ROOT/out"
rm -f "$ESP" "$VARS_COPY"
trap 'rm -f "$VARS_COPY"' EXIT HUP INT TERM
truncate -s 2G "$ESP"
mformat -i "$ESP" -F ::
mmd -i "$ESP" ::/EFI
mmd -i "$ESP" ::/EFI/BOOT
mmd -i "$ESP" ::/boot
mcopy -i "$ESP" "$EFI_APP" ::/EFI/BOOT/BOOTX64.EFI
mcopy -i "$ESP" "$KERNEL" ::/boot/vmlinuz
mcopy -i "$ESP" "$INITRAMFS" ::/boot/initramfs.cpio.gz
cp "$OVMF_VARS" "$VARS_COPY"

echo "Booting Mintosh through its own mBoot UEFI application..."
set +e
timeout --signal=TERM "$TIMEOUT" "$QEMU" \
  -machine pc -m "$MEMORY" -smp 2 \
  -drive "if=pflash,format=raw,unit=0,file=$OVMF_CODE,readonly=on" \
  -drive "if=pflash,format=raw,unit=1,file=$VARS_COPY" \
  -drive "if=ide,index=0,format=raw,file=$ESP" \
  -boot order=c -display none -serial stdio -monitor none -no-reboot > "$LOG" 2>&1
STATUS=$?
set -e
if grep -q "Mintosh Base is ready" "$LOG"; then
  echo "PASS: mBoot loaded the Linux EFI stub and Mintosh reached its ready marker."
  exit 0
fi
echo "FAIL: mBoot did not reach the Mintosh ready marker (QEMU exit: $STATUS)." >&2
tail -n 100 "$LOG" >&2
exit 1

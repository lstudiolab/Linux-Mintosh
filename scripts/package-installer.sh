#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
ISO="$ROOT/out/mintosh-x86_64.iso"
OUT="$ROOT/out"
[ -s "$ISO" ] || { echo "Build the installer ISO first with make iso." >&2; exit 1; }
command -v zip >/dev/null || { echo "zip is required to package the installer." >&2; exit 1; }
(
  cd "$OUT"
  sha256sum mintosh-x86_64.iso > SHA256SUMS
  cat > INSTALL.txt <<'EOF'
Mintosh USB installer

1. Extract this ZIP archive.
2. Open Rufus on Windows.
3. Select mintosh-x86_64.iso as the boot selection.
4. Use GPT for a UEFI computer. Use MBR only for legacy BIOS systems.
5. Start the write operation and allow Rufus to erase the selected USB drive.
6. Restart the laptop and select the USB device from its firmware boot menu.
7. Select Install Mintosh.

The installer requires internet access and an existing internal ext4 partition
with at least 40 GiB free for a temporary local source/build workspace.
The installer does not format the workspace partition. For an erase install,
choose a different target disk from the workspace disk.

Verify the ISO before writing it:
  sha256sum --check SHA256SUMS

This Developer Preview is not hardware-certified. Do not install it on a
machine containing irreplaceable data without a verified backup.
EOF
  rm -f mintosh-installer.zip
  zip -0 -q mintosh-installer.zip mintosh-x86_64.iso SHA256SUMS INSTALL.txt
)
echo "Installer package created: out/mintosh-installer.zip"

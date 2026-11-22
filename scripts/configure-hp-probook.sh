#!/bin/sh
set -eu
[ "$#" -eq 2 ] || { echo "Usage: configure-hp-probook.sh KERNEL_DIR KERNEL_OUT" >&2; exit 64; }
KERNEL_DIR=$1
KERNEL_OUT=$2
CONFIG="$KERNEL_OUT/.config"
CONFIG_TOOL="$KERNEL_DIR/scripts/config"
[ -f "$CONFIG" ] || { echo "Kernel configuration missing: $CONFIG" >&2; exit 1; }
[ -x "$CONFIG_TOOL" ] || { echo "Linux scripts/config is unavailable." >&2; exit 1; }

"$CONFIG_TOOL" --file "$CONFIG" \
  --enable ACPI --enable ACPI_EC --enable ACPI_BATTERY --enable ACPI_BUTTON \
  --enable ACPI_FAN --enable ACPI_THERMAL --enable ACPI_WMI \
  --enable X86_PLATFORM_DEVICES --enable X86_PLATFORM_DRIVERS_HP \
  --enable RFKILL --enable SERIO_I8042 \
  --enable CPU_FREQ --enable CPU_FREQ_GOV_SCHEDUTIL \
  --enable HWMON --enable THERMAL --enable PM --enable PM_SLEEP \
  --enable SUSPEND --enable HIBERNATION --enable FW_LOADER \
  --enable INPUT --enable INPUT_EVDEV --enable I2C --enable HID \
  --enable USB --enable USB_XHCI_HCD --enable USB_XHCI_PCI \
  --enable WLAN --enable CFG80211 --enable MAC80211 \
  --enable BT --enable SND

for module in \
  HP_WMI HP_ACCEL HP_BIOSCFG \
  I2C_HID I2C_HID_ACPI HID_MULTITOUCH MOUSE_ELAN_I2C \
  MOUSE_SYNAPTICS_I2C MOUSE_PS2 MOUSE_PS2_SYNAPTICS \
  USB_VIDEO_CLASS \
  DRM_I915 DRM_AMDGPU DRM_NOUVEAU \
  IWLWIFI IWLMVM RTW88 RTW88_8821CE RTW88_8822BE RTW88_8822CE \
  RTW89 ATH9K ATH10K ATH11K MT76 BRCMFMAC \
  BT_HCIBTUSB BT_HCIBTINTEL BT_HCIBTUSB_RTL BT_HCIBTUSB_MTK \
  SND_HDA_INTEL SND_HDA_CODEC_REALTEK SND_HDA_CODEC_HDMI \
  SND_SOC_SOF SND_SOC_SOF_INTEL_TOPLEVEL SND_SOC_SOF_INTEL_HDA_COMMON \
  SND_SOC_SOF_AMD_TOPLEVEL SND_SOC_AMD_ACP \
  NVME_CORE BLK_DEV_NVME VMD AHCI SATA_AHCI USB_STORAGE USB_UAS \
  THUNDERBOLT TYPEC_UCSI UCSI_ACPI INTEL_PMC_CORE
do
  "$CONFIG_TOOL" --file "$CONFIG" --module "$module"
done

make -C "$KERNEL_DIR" O="$(cd "$KERNEL_OUT" && pwd)" ARCH=x86 olddefconfig
echo "HP ProBook multi-generation driver profile applied."

#!/usr/bin/env bash
set -Eeuo pipefail

export TERM=${TERM:-linux}
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
. "$SCRIPT_DIR/installer-disk-safety.sh"
. "$SCRIPT_DIR/installer-storage.sh"
TITLE="Mintosh Installer"
TARGET=/mnt/mintosh
LOG=/var/log/mintosh-installer.log
mkdir -p "$(dirname "$LOG")"

fail() {
  dialog --title "$TITLE" --msgbox "$1\n\nDetails: $LOG" 12 72 || true
  exit 1
}
cleanup() {
  if mountpoint -q "$TARGET/dev"; then umount -R "$TARGET/dev" || true; fi
  if mountpoint -q "$TARGET"; then umount -R "$TARGET" || true; fi
}
trap cleanup EXIT

command -v dialog >/dev/null || { echo "dialog is required" >&2; exit 1; }
dialog --clear --title "$TITLE" --msgbox "Welcome to Mintosh.\n\nYou are running the live environment. No disk has been changed.\n\nChoose whether to install Mintosh or try the live system first." 12 70

ACTION=$(dialog --stdout --clear --title "$TITLE" --menu "What would you like to do?" 15 72 5 \
  install "Install Mintosh" \
  try "Try Mintosh without installing" \
  reboot "Restart computer" \
  poweroff "Shut down") || exit 0

case "$ACTION" in
  try)
    dialog --title "$TITLE" --msgbox "You are now using the Mintosh live system.\n\nYour internal disks have not been modified. Close this terminal to return to the installer." 10 70
    /bin/bash -l
    exec "$0"
    ;;
  reboot) reboot ;;
  poweroff) poweroff ;;
  install) ;;
esac

MODE=$(dialog --stdout --title "$TITLE" --menu "Choose an installation type." 15 76 4 \
  alongside "Install alongside an existing operating system" \
  erase "Erase a disk and install Mintosh") || exit 0

mintosh_discover_protected_disks || fail "Could not identify the live Mintosh boot medium. Disk selection is disabled to protect the installer USB."
DISK_LIST=$(mintosh_list_installable_disks) || fail "No safe installable disks were detected."
mapfile -t DISKS <<<"$DISK_LIST"
(( ${#DISKS[@]} > 0 )) || fail "No installable disks were detected."
MENU=()
for item in "${DISKS[@]}"; do
  dev=${item%% *}
  label=${item#* }
  MENU+=("$dev" "$label")
done
DISK=$(dialog --stdout --title "$TITLE" --menu "Select the destination disk. The live boot medium and removable devices are hidden." 18 78 8 "${MENU[@]}") || exit 0
[[ -b "$DISK" ]] || fail "The selected disk is no longer available."
mintosh_disk_is_protected "$DISK" && fail "The selected disk contains the live Mintosh boot medium. It is protected from installation."
[[ "$(lsblk -dnro TYPE "$DISK")" == "disk" ]] || fail "Only whole physical disks can be selected."

if [[ "$MODE" == erase ]]; then
  dialog --title "Erase disk warning" --yesno "ALL DATA AND ALL OPERATING SYSTEMS ON:\n\n$DISK\n\nWILL BE PERMANENTLY DELETED.\n\nThis cannot be undone. Continue?" 13 76 || exit 0
  CONFIRM=$(dialog --stdout --title "Final confirmation" --inputbox "Type ERASE to confirm that you want to erase $DISK." 10 72) || exit 0
  [[ "$CONFIRM" == "ERASE" ]] || fail "Confirmation did not match. No disk changes were made."
else
  dialog --title "Dual-boot requirements" --msgbox "Mintosh will only use unallocated free space on $DISK. It will not shrink, format, or overwrite existing partitions.\n\nBefore continuing, use your existing operating system's disk manager to create at least 20 GiB of unallocated space.\n\nAn existing EFI System Partition is required when this computer was booted in UEFI mode." 15 76
fi

command -v grub-install >/dev/null && command -v update-grub >/dev/null && command -v update-initramfs >/dev/null || fail "Required bootloader tools are missing from this live image."

read -r USERNAME < <(dialog --stdout --title "$TITLE" --inputbox "Choose your Mintosh account username." 9 60 mintosh) || exit 0
[[ "$USERNAME" =~ ^[a-z_][a-z0-9_-]*$ ]] || fail "Invalid username."
if [[ "$USERNAME" != "mintosh" ]]; then
  getent passwd "$USERNAME" >/dev/null && fail "That username already exists."
  getent group "$USERNAME" >/dev/null && fail "That username conflicts with an existing group."
fi
PASSWORD=$(dialog --stdout --insecure --title "$TITLE" --passwordbox "Choose a password for $USERNAME." 9 60) || exit 0
[[ "${#PASSWORD}" -ge 8 ]] || fail "Use a password with at least 8 characters."
PASSWORD2=$(dialog --stdout --insecure --title "$TITLE" --passwordbox "Enter the password again." 9 60) || exit 0
[[ "$PASSWORD" == "$PASSWORD2" ]] || fail "Passwords do not match."

UEFI=0
[[ -d /sys/firmware/efi ]] && UEFI=1
ROOT_PART=""
EFI_PART=""

if [[ "$MODE" == erase ]]; then
  parted -s "$DISK" mklabel gpt >>"$LOG" 2>&1 || fail "Could not create a new GPT partition table."
  if (( UEFI )); then
    parted -s "$DISK" mkpart ESP fat32 1MiB 1025MiB >>"$LOG" 2>&1
    parted -s "$DISK" set 1 esp on >>"$LOG" 2>&1
    parted -s "$DISK" mkpart primary ext4 1025MiB 100% >>"$LOG" 2>&1
  
  else
    parted -s "$DISK" mkpart BIOSBOOT 1MiB 3MiB >>"$LOG" 2>&1
    parted -s "$DISK" set 1 bios_grub on >>"$LOG" 2>&1
    parted -s "$DISK" mkpart primary ext4 3MiB 100% >>"$LOG" 2>&1
  fi
  partprobe "$DISK" || true
  udevadm settle || true
  if (( UEFI )); then
    EFI_PART=$(lsblk -nrpo NAME "$DISK" | sed -n '2p')
    ROOT_PART=$(lsblk -nrpo NAME "$DISK" | sed -n '3p')
  else
    ROOT_PART=$(lsblk -nrpo NAME "$DISK" | sed -n '3p')
  fi
else
  if (( UEFI )); then
    EFI_PART=$(lsblk -nrpo NAME,PARTTYPE,PARTFLAGS "$DISK" | awk 'tolower($0) ~ /c12a7328-f81f-11d2-ba4b-00a0c93ec93b|esp/ {print $1; exit}')
    [[ -n "$EFI_PART" ]] || fail "No EFI System Partition was found. Boot Mintosh in the same firmware mode as the installed operating system, or create an EFI System Partition first."
  elif [[ "$(lsblk -dnro PTTYPE "$DISK")" == "gpt" ]]; then
    parted -s "$DISK" print | grep -qi bios_grub || fail "This GPT disk has no BIOS boot partition. Boot Mintosh in UEFI mode or prepare a BIOS boot partition first."
  fi
  FREE=$(mintosh_find_largest_free_region "$DISK" 20480 2>/dev/null || true)
  [[ -n "$FREE" ]] || fail "No unallocated region of at least 20 GiB was found. No existing partitions were changed."
  BEFORE=$(lsblk -nrpo NAME "$DISK")
  read -r START END <<<"$FREE"
  parted -s "$DISK" mkpart primary ext4 "$START" "$END" >>"$LOG" 2>&1 || fail "Could not create a Mintosh partition in the free space."
  partprobe "$DISK" || true
  udevadm settle || true
  while IFS= read -r part; do
    if ! grep -Fxq "$part" <<<"$BEFORE"; then ROOT_PART="$part"; fi
  done < <(lsblk -nrpo NAME "$DISK")
fi

[[ -b "$ROOT_PART" ]] || fail "Could not identify the new Mintosh root partition."
if (( UEFI )) && [[ "$MODE" == erase ]]; then
  [[ -b "$EFI_PART" ]] || fail "Could not identify the new EFI partition."
  mkfs.fat -F32 "$EFI_PART" >>"$LOG" 2>&1 || fail "Could not format the EFI partition."
fi
mkfs.ext4 -F -L Mintosh "$ROOT_PART" >>"$LOG" 2>&1 || fail "Could not format the Mintosh root partition."

dialog --title "$TITLE" --infobox "Preparing the Mintosh filesystem..." 5 58
mkdir -p "$TARGET"
mount "$ROOT_PART" "$TARGET" || fail "Could not mount the new root partition."
mkdir -p "$TARGET/boot/efi"
if (( UEFI )); then
  mount "$EFI_PART" "$TARGET/boot/efi" || fail "Could not mount the EFI System Partition."
fi

dialog --title "$TITLE" --infobox "Installing Mintosh system files. This can take several minutes..." 5 72
rsync -aHAXx --numeric-ids \
  --exclude=/dev/*** --exclude=/proc/*** --exclude=/sys/*** --exclude=/run/*** \
  --exclude=/tmp/*** --exclude=/mnt/*** --exclude=/media/*** --exclude=/lost+found \
  --exclude=/init / "$TARGET/" >>"$LOG" 2>&1 || fail "Could not copy the Mintosh base system."

rm -f "$TARGET/etc/systemd/system/multi-user.target.wants/mintosh-installer.service"
ROOT_UUID=$(blkid -s UUID -o value "$ROOT_PART")
if (( UEFI )); then
  EFI_UUID=$(blkid -s UUID -o value "$EFI_PART")
  printf 'UUID=%s / ext4 defaults,noatime 0 1\nUUID=%s /boot/efi vfat umask=0077 0 1\n' "$ROOT_UUID" "$EFI_UUID" > "$TARGET/etc/fstab"
else
  printf 'UUID=%s / ext4 defaults,noatime 0 1\n' "$ROOT_UUID" > "$TARGET/etc/fstab"
fi
: > "$TARGET/etc/machine-id"
rm -f "$TARGET/var/lib/dbus/machine-id"
ln -s /etc/machine-id "$TARGET/var/lib/dbus/machine-id"
ln -snf /run/systemd/resolve/stub-resolv.conf "$TARGET/etc/resolv.conf"
printf '%s\n' mintosh > "$TARGET/etc/hostname"

for dir in dev proc sys run; do mount --rbind "/$dir" "$TARGET/$dir"; mount --make-rslave "$TARGET/$dir"; done
if [[ "$USERNAME" != "mintosh" ]]; then
  chroot "$TARGET" groupmod -n "$USERNAME" mintosh >>"$LOG" 2>&1 || fail "Could not rename the primary user group."
  chroot "$TARGET" usermod -l "$USERNAME" mintosh >>"$LOG" 2>&1 || fail "Could not configure the user account."
fi
chroot "$TARGET" usermod -d "/home/$USERNAME" -m "$USERNAME" >>"$LOG" 2>&1 || fail "Could not configure the home directory."
printf '%s:%s\n' "$USERNAME" "$PASSWORD" | chroot "$TARGET" chpasswd || fail "Could not set the account password."
chroot "$TARGET" usermod -aG sudo,mintosh-services "$USERNAME" >>"$LOG" 2>&1 || fail "Could not configure administrator and Mintosh service permissions."
unset PASSWORD PASSWORD2

if (( UEFI )); then
  dialog --title "$TITLE" --infobox "Installing UEFI bootloader..." 5 58
  if ! chroot "$TARGET" grub-install --target=x86_64-efi --efi-directory=/boot/efi --bootloader-id=Mintosh --recheck >>"$LOG" 2>&1; then
    chroot "$TARGET" grub-install --target=x86_64-efi --efi-directory=/boot/efi --removable --no-nvram --recheck >>"$LOG" 2>&1 || fail "UEFI bootloader installation failed."
  fi
else
  dialog --title "$TITLE" --infobox "Installing BIOS bootloader..." 5 58
  chroot "$TARGET" grub-install --target=i386-pc "$DISK" >>"$LOG" 2>&1 || fail "BIOS bootloader installation failed."
fi

if [[ -f "$TARGET/etc/default/grub" ]]; then
  sed -i 's/^#\?GRUB_DISABLE_OS_PROBER=.*/GRUB_DISABLE_OS_PROBER=false/' "$TARGET/etc/default/grub"
  grep -q '^GRUB_DISABLE_OS_PROBER=' "$TARGET/etc/default/grub" || echo 'GRUB_DISABLE_OS_PROBER=false' >> "$TARGET/etc/default/grub"
fi
chroot "$TARGET" update-initramfs -c -k all >>"$LOG" 2>&1 || chroot "$TARGET" update-initramfs -u -k all >>"$LOG" 2>&1
chroot "$TARGET" update-grub >>"$LOG" 2>&1 || fail "Could not generate the boot menu."
sync
if dialog --title "Installation complete" --yesno "Mintosh has been installed on $DISK.\n\nRemove the USB installer and restart.\n\nRestart now?" 12 72; then
  cleanup
  trap - EXIT
  reboot
fi
dialog --title "$TITLE" --msgbox "Installation completed. Remove the USB drive and restart when ready." 8 66

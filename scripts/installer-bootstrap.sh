#!/usr/bin/env bash
set -Eeuo pipefail
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
. "$SCRIPT_DIR/installer-disk-safety.sh"
SOURCE_ENV=/etc/mintosh/installer-source.env
RUNTIME_ENV=/run/mintosh-build.env
WORKSPACE_MOUNT=/mnt/mintosh-workspace
LOG=/var/log/mintosh-bootstrap.log
REPOSITORY=https://github.com/lstudiolab/Linux-Mintosh.git
mkdir -p /run "$WORKSPACE_MOUNT"

mintosh_bootstrap_fail() {
  printf 'Mintosh bootstrap: %s\n' "$1" | tee -a "$LOG" >&2
  dialog --title "Mintosh Installer" --msgbox "$1\n\nNo installation disk has been modified.\n\nLog: $LOG" 12 74 || true
  return 1
}

mintosh_prepare_local_build() {
  : >"$LOG"
  [ -r "$SOURCE_ENV" ] || { mintosh_bootstrap_fail "Installer source pin is missing. Rebuild the official ISO from a Git checkout."; return 1; }
  . "$SOURCE_ENV"
  [ "$MINTOSH_SOURCE_REPOSITORY" = "$REPOSITORY" ] || { mintosh_bootstrap_fail "Unexpected source repository."; return 1; }
  [[ "$MINTOSH_SOURCE_COMMIT" =~ ^[0-9a-f]{40}$ ]] || { mintosh_bootstrap_fail "Invalid pinned source commit."; return 1; }

  if ! curl -fsSL --connect-timeout 8 --max-time 20 https://github.com/ -o /dev/null; then
    dialog --title "Internet required" --msgbox "Connect to Wi-Fi or Ethernet. Mintosh must download its source and build dependencies on this laptop." 9 70
    command -v nmtui >/dev/null && nmtui
    curl -fsSL --connect-timeout 8 --max-time 20 https://github.com/ -o /dev/null || { mintosh_bootstrap_fail "Internet access is unavailable."; return 1; }
  fi

  mintosh_discover_protected_disks || { mintosh_bootstrap_fail "Cannot identify and protect the installer USB."; return 1; }
  local line part type fstype size removable parent model parent_rm count=0
  local -a menu=()
  while IFS= read -r line; do
    read -r part type fstype size removable parent model <<<"$line"
    [ "$type" = part ] && [ "$fstype" = ext4 ] && [ "$removable" = 0 ] && [ -n "$parent" ] || continue
    parent="/dev/$parent"
    mintosh_disk_is_protected "$parent" && continue
    parent_rm=$(lsblk -dnro RM "$parent" 2>/dev/null || echo 1)
    [ "$parent_rm" = 0 ] || continue
    findmnt -rn -S "$part" >/dev/null 2>&1 && continue
    count=$((count+1))
    menu+=("$part" "$size ext4 $model")
  done < <(lsblk -rpn -o NAME,TYPE,FSTYPE,SIZE,RM,PKNAME,MODEL)
  [ "$count" -gt 0 ] || { mintosh_bootstrap_fail "An unused internal ext4 partition is required for a temporary build workspace with 40 GiB free. No partition will be formatted to create one."; return 1; }

  local selected
  selected=$(dialog --stdout --title "Temporary build storage" --menu "Choose an ext4 partition on internal or external storage with at least 40 GiB free. Mintosh will create a temporary hidden directory and will not format this partition." 20 78 8 "${menu[@]}") || return 1
  mount -o rw,noatime "$selected" "$WORKSPACE_MOUNT" >>"$LOG" 2>&1 || { mintosh_bootstrap_fail "Cannot mount the selected workspace."; return 1; }
  local free_kib
  free_kib=$(df -Pk "$WORKSPACE_MOUNT" | awk 'NR==2 {print $4}')
  [ "$free_kib" -ge 41943040 ] || { umount "$WORKSPACE_MOUNT"; mintosh_bootstrap_fail "The workspace needs at least 40 GiB free."; return 1; }

  MINTOSH_WORKSPACE_PARTITION=$selected
  MINTOSH_WORKSPACE_DISK="/dev/$(lsblk -dnro PKNAME "$selected")"
  MINTOSH_WORKSPACE_DIR=$(mktemp -d "$WORKSPACE_MOUNT/.mintosh-build.XXXXXX")
  chmod 0700 "$MINTOSH_WORKSPACE_DIR"
  local source_dir="$MINTOSH_WORKSPACE_DIR/source"
  git init "$source_dir" >>"$LOG" 2>&1
  git -C "$source_dir" remote add origin "$MINTOSH_SOURCE_REPOSITORY"
  git -C "$source_dir" fetch --depth=1 origin "$MINTOSH_SOURCE_COMMIT" >>"$LOG" 2>&1 || { mintosh_bootstrap_fail "Could not fetch the pinned Mintosh source revision."; return 1; }
  git -C "$source_dir" checkout --detach FETCH_HEAD >>"$LOG" 2>&1
  [ "$(git -C "$source_dir" rev-parse HEAD)" = "$MINTOSH_SOURCE_COMMIT" ] || { mintosh_bootstrap_fail "Source commit verification failed."; return 1; }
  MINTOSH_SOURCE_DIR=$source_dir

  "$SCRIPT_DIR/mintosh-hardware-audit" --workspace "$WORKSPACE_MOUNT" 2>&1 | tee -a "$LOG" || { mintosh_bootstrap_fail "A required hardware or build check failed. Installation is blocked."; return 1; }

  mintosh_run_step() {
    local label=$1
    shift
    dialog --title "Mintosh Installer" --infobox "$label\n\nBuilding locally. Keep the laptop connected to power." 6 72
    (cd "$MINTOSH_SOURCE_DIR" && "$@") >>"$LOG" 2>&1 || { mintosh_bootstrap_fail "$label failed. No target disk has been modified."; return 1; }
  }
  mintosh_run_step "Verify source and native foundation" make verify || return 1
  mintosh_run_step "Download pinned Linux kernel" make kernel-fetch || return 1
  mintosh_run_step "Configure generic x86-64 hardware" make kernel-config MINTOSH_HARDWARE_PROFILE=generic || return 1
  mintosh_run_step "Build Linux kernel and modules" make kernel JOBS=2 || return 1
  mintosh_run_step "Build Mintosh root filesystem" make kernel-install JOBS=2 || return 1
  mintosh_run_step "Build bootable verification image" make iso JOBS=2 || return 1
  mintosh_run_step "Boot-test the image in QEMU" make test-qemu QEMU_MEMORY=4096 QEMU_TIMEOUT=180 || return 1

  MINTOSH_BUILD_ROOTFS="$MINTOSH_SOURCE_DIR/out/rootfs"
  [ -s "$MINTOSH_BUILD_ROOTFS/etc/os-release" ] || { mintosh_bootstrap_fail "The build did not produce a Mintosh root filesystem."; return 1; }
  compgen -G "$MINTOSH_BUILD_ROOTFS/boot/vmlinuz-*" >/dev/null || { mintosh_bootstrap_fail "The build did not produce a kernel."; return 1; }
  compgen -G "$MINTOSH_BUILD_ROOTFS/lib/modules/*" >/dev/null || { mintosh_bootstrap_fail "The build did not produce kernel modules."; return 1; }

  {
    printf 'MINTOSH_BUILD_ROOTFS=%q\n' "$MINTOSH_BUILD_ROOTFS"
    printf 'MINTOSH_BUILD_COMMIT=%q\n' "$MINTOSH_SOURCE_COMMIT"
    printf 'MINTOSH_WORKSPACE_MOUNT=%q\n' "$WORKSPACE_MOUNT"
    printf 'MINTOSH_WORKSPACE_PARTITION=%q\n' "$MINTOSH_WORKSPACE_PARTITION"
    printf 'MINTOSH_WORKSPACE_DISK=%q\n' "$MINTOSH_WORKSPACE_DISK"
    printf 'MINTOSH_WORKSPACE_DIR=%q\n' "$MINTOSH_WORKSPACE_DIR"
  } >"$RUNTIME_ENV"
  chmod 0600 "$RUNTIME_ENV"
  dialog --title "Verification passed" --msgbox "The pinned source was built on this laptop. All required hardware checks passed and the locally built image passed QEMU startup verification.\n\nNo installation disk has been modified. You can now choose the installation target." 12 74
}


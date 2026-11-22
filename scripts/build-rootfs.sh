#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
ROOTFS="$ROOT/out/rootfs"
RELEASE="$ROOT/configs/release.json"
SUITE=$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1]))["debian_suite"])' "$RELEASE")
VERSION=$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1]))["release"])' "$RELEASE")
TARGET_USER=${MINTOSH_USER:-mintosh}
[ "$TARGET_USER" = "mintosh" ] || { echo "The installer currently requires the live account name mintosh." >&2; exit 1; }
TARGET_PASSWORD=${MINTOSH_PASSWORD:-}
ARCH=${ARCH:-x86_64}
[ "$ARCH" = x86_64 ] || { echo "The Mintosh root filesystem currently targets x86_64." >&2; exit 1; }
[ "$(id -u)" -eq 0 ] || { echo "Run through sudo: make rootfs" >&2; exit 1; }
command -v debootstrap >/dev/null || { echo "Install debootstrap (make host-deps)." >&2; exit 1; }
case "$TARGET_USER" in [a-z_]* ) ;; *) echo "MINTOSH_USER must begin with a letter or underscore." >&2; exit 1 ;; esac
case "$TARGET_USER" in *[!a-zA-Z0-9_-]*|'') echo "Invalid MINTOSH_USER." >&2; exit 1 ;; esac
[ ${#TARGET_USER} -le 32 ] || { echo "MINTOSH_USER is too long (maximum 32 characters)." >&2; exit 1; }
mkdir -p "$ROOT/out"
if [ -e "$ROOTFS/etc/os-release" ]; then
  echo "Root filesystem already exists at $ROOTFS (run make clean to rebuild it)."
  exit 0
fi
rm -rf "$ROOTFS"
echo "Creating Mintosh $VERSION root filesystem from Debian $SUITE..."
debootstrap --arch=amd64 --variant=minbase --components=main,contrib,non-free,non-free-firmware \
  --include=systemd,systemd-sysv,systemd-resolved,udev,dbus,network-manager,wpasupplicant,wireless-regdb,kmod,bash,coreutils,util-linux,procps,findutils,grep,sed,gawk,tar,gzip,xz-utils,iproute2,iputils-ping,netbase,ca-certificates,sudo,passwd,login,locales,tzdata,e2fsprogs,less,nano,libsqlite3-0,libsqlite3-dev,g++,curl,wget,openssh-client,dialog,parted,dosfstools,rsync,os-prober,efibootmgr,ntfs-3g,initramfs-tools,grub-common,grub-pc-bin,grub-efi-amd64-bin,python3,grub2-common,gcc,make,libc6-dev,firmware-iwlwifi,firmware-realtek,firmware-atheros,firmware-brcm80211,firmware-mediatek,firmware-amd-graphics,firmware-intel-graphics,firmware-intel-sound,firmware-sof-signed,firmware-misc-nonfree,firmware-nvidia-graphics,bluez,rfkill,fwupd,pciutils,usbutils,acpi,iw,ethtool \
  "$SUITE" "$ROOTFS" https://deb.debian.org/debian
printf '%s\n' mintosh > "$ROOTFS/etc/hostname"
cat > "$ROOTFS/etc/hosts" <<'EOF'
127.0.0.1 localhost
127.0.1.1 mintosh
::1 localhost ip6-localhost ip6-loopback
EOF
cat > "$ROOTFS/etc/os-release" <<EOF
NAME="Mintosh"
PRETTY_NAME="Mintosh $VERSION"
ID=mintosh
ID_LIKE=debian
VERSION_ID="$VERSION"
VERSION_CODENAME="$SUITE"
HOME_URL="https://github.com/lstudiolab/Linux-Mintosh"
EOF
printf 'Mintosh %s \\n \\l\n' "$VERSION" > "$ROOTFS/etc/issue"
cat > "$ROOTFS/etc/mintosh-release" <<EOF
MINTOSH_VERSION="$VERSION"
MINTOSH_DEBIAN_SUITE="$SUITE"
EOF
cat > "$ROOTFS/etc/fstab" <<'EOF'
proc /proc proc defaults 0 0
sysfs /sys sysfs defaults 0 0
devtmpfs /dev devtmpfs mode=0755,nosuid 0 0
tmpfs /run tmpfs mode=0755,nosuid,nodev 0 0
EOF
mkdir -p "$ROOTFS/etc/NetworkManager/conf.d" "$ROOTFS/etc/systemd/system" "$ROOTFS/usr/lib/systemd/user" "$ROOTFS/etc/systemd/user/default.target.wants"
cat > "$ROOTFS/etc/NetworkManager/conf.d/10-mintosh.conf" <<'EOF'
[main]
dns=systemd-resolved

[device]
wifi.scan-rand-mac-address=yes
EOF
mkdir -p "$ROOTFS/usr/src/mintosh" "$ROOTFS/etc/mintosh/services"
cp -a "$ROOT/Sources" "$ROOTFS/usr/src/mintosh/"
chroot "$ROOTFS" make -C /usr/src/mintosh/Sources/MintoshCore install
chroot "$ROOTFS" make -C /usr/src/mintosh/Sources/XCM install
rm -rf "$ROOTFS/usr/src/mintosh"
chroot "$ROOTFS" apt-mark manual libsqlite3-0 libstdc++6
chroot "$ROOTFS" apt-get purge --yes gcc g++ make libc6-dev libsqlite3-dev
chroot "$ROOTFS" apt-get autoremove --yes
chroot "$ROOTFS" apt-get clean
install -m 0644 "$ROOT/Sources/Services/launch7762.service" "$ROOTFS/etc/systemd/system/launch7762.service"
install -m 0644 "$ROOT/Sources/Services/mintosh-ready.service" "$ROOTFS/etc/systemd/system/mintosh-ready.service"
install -m 0644 "$ROOT/Sources/Services/xcm.service" "$ROOTFS/etc/systemd/system/xcm.service"
install -m 0644 "$ROOT/Sources/Services/ioplus-events.service" "$ROOTFS/etc/systemd/system/ioplus-events.service"
install -m 0644 "$ROOT/Sources/Services/launch-client.service" "$ROOTFS/usr/lib/systemd/user/launch-client.service"
ln -sfn /usr/lib/systemd/user/launch-client.service "$ROOTFS/etc/systemd/user/default.target.wants/launch-client.service"
install -m 0644 "$ROOT/Sources/Services/darl.service" "$ROOTFS/etc/mintosh/services/darl.service"
cat > "$ROOTFS/etc/systemd/system/mintosh-installer.service" <<'EOF'
[Unit]
Description=Mintosh live installer
After=systemd-user-sessions.service
Conflicts=getty@tty1.service
[Service]
Type=simple
ExecStart=/usr/bin/mintosh-installer
StandardInput=tty
StandardOutput=tty
StandardError=tty
TTYPath=/dev/tty1
TTYReset=yes
TTYVHangup=yes
TTYVTDisallocate=yes
Restart=on-failure
RestartSec=2
[Install]
WantedBy=multi-user.target
EOF
ln -snf /usr/local/sbin/mintosh-installer "$ROOTFS/usr/bin/mintosh-installer"
cat > "$ROOTFS/etc/motd" <<'EOF'
Welcome to Mintosh.
A Linux system designed with a simple, consistent desktop experience.
EOF
chroot "$ROOTFS" groupadd --system mintosh
chroot "$ROOTFS" groupadd --system mintosh-services
mkdir -p "$ROOTFS/var/lib/mintosh"
chroot "$ROOTFS" useradd --create-home --gid mintosh --shell /bin/bash --groups sudo,mintosh-services "$TARGET_USER"
if [ -n "$TARGET_PASSWORD" ]; then
  printf '%s:%s\n' "$TARGET_USER" "$TARGET_PASSWORD" | chroot "$ROOTFS" chpasswd
else
  chroot "$ROOTFS" passwd --lock "$TARGET_USER"
  echo "Created $TARGET_USER with a locked password. Set MINTOSH_PASSWORD when rebuilding to enable login."
fi
chroot "$ROOTFS" passwd --lock root
chroot "$ROOTFS" locale-gen en_US.UTF-8 || true
chroot "$ROOTFS" update-locale LANG=en_US.UTF-8 || true
ln -snf /usr/share/zoneinfo/UTC "$ROOTFS/etc/localtime"
: > "$ROOTFS/etc/machine-id"
mkdir -p "$ROOTFS/var/lib/dbus"
ln -snf /etc/machine-id "$ROOTFS/var/lib/dbus/machine-id"
systemctl --root="$ROOTFS" enable NetworkManager.service systemd-resolved.service serial-getty@ttyS0.service launch7762.service mintosh-ready.service xcm.service mintosh-installer.service ioplus-events.service
ln -snf /run/systemd/resolve/stub-resolv.conf "$ROOTFS/etc/resolv.conf"
rm -rf "$ROOTFS/var/lib/apt/lists/"*
echo "Mintosh root filesystem created at out/rootfs."

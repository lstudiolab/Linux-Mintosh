SHELL := /bin/sh
ARCH ?= x86_64
export ARCH MINTOSH_USER MINTOSH_PASSWORD
JOBS ?= $(shell getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)
KERNEL_DIR := kernel/linux
KERNEL_OUT := out/kernel/$(ARCH)
ROOTFS := out/rootfs
ISO_ROOT := out/iso
ISO := out/mintosh-$(ARCH).iso
KERNEL_ARCH_x86_64 := x86
KERNEL_ARCH_arm64 := arm64
KERNEL_DEFCONFIG_x86_64 := x86_64_defconfig
KERNEL_DEFCONFIG_arm64 := defconfig
KERNEL_DEFCONFIG := $(KERNEL_DEFCONFIG_$(ARCH))
KERNEL_ARCH := $(KERNEL_ARCH_$(ARCH))
ifeq ($(ARCH),arm64)
CROSS_COMPILE ?= aarch64-linux-gnu-
KERNEL_IMAGE := Image
else
KERNEL_IMAGE := bzImage
endif

.PHONY: help check-host host-deps kernel-fetch kernel-config kernel kernel-install rootfs initramfs iso image qemu test-qemu usb clean

help:
	@printf '%s\n' \
	  'Mintosh Base targets:' \
	  '  make host-deps       Install Debian/Ubuntu build dependencies' \
	  '  make kernel-fetch    Fetch the release-pinned upstream Linux kernel' \
	  '  make kernel-config   Configure the kernel for Mintosh/QEMU' \
	  '  make kernel          Build the kernel and modules' \
	  '  make rootfs          Build the Debian-based Mintosh root filesystem' \
	  '  make initramfs       Package the root filesystem as an initramfs' \
	  '  make iso             Build a bootable BIOS/UEFI GRUB ISO' \
	  '  make qemu            Boot the ISO in QEMU interactively' \
	  '  make test-qemu       Run a serial-console boot smoke test' \
	  '  make usb USB=/dev/sdX  Write ISO to USB (erases that drive)' \
	  '  make clean           Remove generated output' \
	  '' \
	  'Options: ARCH=x86_64|arm64 JOBS=<parallel jobs>' \
	  'Full ISO/QEMU image: ARCH=x86_64 (ARM64 kernel builds are supported separately).'

check-host:
	@command -v make >/dev/null || { echo 'Missing dependency: make' >&2; exit 1; }
	@command -v python3 >/dev/null || { echo 'Missing dependency: python3' >&2; exit 1; }
	@command -v curl >/dev/null || { echo 'Missing dependency: curl' >&2; exit 1; }
	@case "$(ARCH)" in x86_64|arm64) ;; *) echo 'Unsupported ARCH: $(ARCH)' >&2; exit 1 ;; esac

host-deps:
	@sudo apt-get update
	@sudo apt-get install -y bc bison build-essential cpio curl debootstrap flex gcc g++ \
	  grub-common grub-pc-bin grub-efi-amd64-bin kmod libelf-dev libncurses-dev \
	  libssl-dev make mtools python3 qemu-system-x86 rsync gcc-aarch64-linux-gnu binutils-aarch64-linux-gnu xorriso xz-utils

kernel-fetch: check-host
	@ARCH="$(ARCH)" sh ./scripts/fetch-kernel.sh

kernel-config: check-host
	@test -d "$(KERNEL_DIR)" || { echo 'Run make kernel-fetch first.' >&2; exit 1; }
	@mkdir -p "$(KERNEL_OUT)"
	$(MAKE) -C "$(KERNEL_DIR)" O="$(CURDIR)/$(KERNEL_OUT)" ARCH="$(KERNEL_ARCH)" CROSS_COMPILE="$(CROSS_COMPILE)" $(KERNEL_DEFCONFIG)
	@"$(KERNEL_DIR)/scripts/config" --file "$(KERNEL_OUT)/.config" \
	  --enable MODULES --enable BLK_DEV_INITRD --enable DEVTMPFS --enable DEVTMPFS_MOUNT \
	  --enable EXT4_FS --enable VIRTIO --enable VIRTIO_PCI --enable VIRTIO_BLK \
	  --enable VIRTIO_NET --enable SERIAL_8250 --enable SERIAL_8250_CONSOLE \
	  --enable EFI --enable EFI_STUB --enable EFI_PARTITION --enable EFI_VARS --enable FAT_FS --enable VFAT_FS --enable MSDOS_PARTITION --enable PROC_FS --enable SYSFS --enable TMPFS \
	  --set-str LOCALVERSION -mintosh
	$(MAKE) -C "$(KERNEL_DIR)" O="$(CURDIR)/$(KERNEL_OUT)" ARCH="$(KERNEL_ARCH)" CROSS_COMPILE="$(CROSS_COMPILE)" olddefconfig

kernel: check-host
	@test -f "$(KERNEL_OUT)/.config" || { echo 'Run make kernel-config first.' >&2; exit 1; }
	$(MAKE) -C "$(KERNEL_DIR)" O="$(CURDIR)/$(KERNEL_OUT)" ARCH="$(KERNEL_ARCH)" CROSS_COMPILE="$(CROSS_COMPILE)" -j"$(JOBS)" $(KERNEL_IMAGE) modules
	@echo 'Kernel build completed: $(KERNEL_OUT)'

kernel-install: kernel
	@test "$(ARCH)" = x86_64 || { echo 'Rootfs/ISO packaging currently targets x86_64.' >&2; exit 1; }
	@sudo mkdir -p "$(ROOTFS)/lib/modules" "$(ROOTFS)/boot"
	@sudo $(MAKE) -C "$(KERNEL_DIR)" O="$(CURDIR)/$(KERNEL_OUT)" ARCH="$(KERNEL_ARCH)" INSTALL_MOD_PATH="$(CURDIR)/$(ROOTFS)" modules_install
	@sudo install -m 0644 "$(KERNEL_OUT)/arch/x86/boot/bzImage" "$(ROOTFS)/boot/vmlinuz-mintosh"

rootfs: check-host
	@test "$(ARCH)" = x86_64 || { echo 'Rootfs/ISO packaging currently targets x86_64.' >&2; exit 1; }
	@sudo --preserve-env=ARCH,MINTOSH_USER,MINTOSH_PASSWORD sh ./scripts/build-rootfs.sh

initramfs: rootfs kernel-install
	@sudo sh ./scripts/build-initramfs.sh

iso: initramfs
	@sudo sh ./scripts/build-iso.sh

image: iso

qemu: iso
	qemu-system-x86_64 -m 2048 -smp 2 -cdrom "$(ISO)" -boot d

test-qemu: iso
	@sh ./scripts/test-qemu.sh "$(ISO)"

usb: iso
	@test -n "$(USB)" || { echo 'Specify USB=/dev/sdX' >&2; exit 1; }
	sudo sh ./scripts/write-usb.sh "$(ISO)" "$(USB)"

clean:
	@sudo rm -rf out
	@echo 'Removed Mintosh build output.'

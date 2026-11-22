SHELL := /bin/sh
ARCH ?= x86_64
MINTOSH_HARDWARE_PROFILE ?= generic
QEMU_MEMORY ?= 4096
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

MBOOT_OUT := out/mboot
EFI_CRT := $(firstword $(wildcard /usr/lib/crt0-efi-x86_64.o /usr/lib/gnuefi/crt0-efi-x86_64.o))
EFI_LDS := $(firstword $(wildcard /usr/lib/elf_x86_64_efi.lds /usr/lib/gnuefi/elf_x86_64_efi.lds))
EFI_LIBDIR := $(dir $(EFI_CRT))

.PHONY: help check-makefiles test-native-core test-kernel-config test-installer-safety test-mboot mboot check-host check-release check-scripts verify release-info host-deps kernel-fetch kernel-config kernel kernel-install rootfs initramfs iso image qemu test-qemu usb clean

help:
	@printf '%s\\n' \
	  'Mintosh release and build targets:' \
	  '  make check-release  Validate release manifest and architecture pins' \
	  '  make check-scripts  Validate shell scripts' \
	  '  make verify         Run all offline validation' \
	  '  make mboot          Build the native x86-64 UEFI bootloader' \
	  '  make release-info   Print release and kernel versions' \
	  '  make host-deps      Install Debian/Ubuntu build dependencies' \
	  '  make kernel-fetch   Fetch the release-pinned upstream Linux kernel' \
	  '  make kernel-config  Configure Linux for Mintosh/QEMU' \
	  '  make kernel         Build the kernel and modules' \
	  '  make rootfs         Build the Debian-based Mintosh root filesystem' \
	  '  make initramfs      Package the live root filesystem as initramfs' \
	  '  make iso            Build the bootable BIOS/UEFI GRUB ISO' \
	  '  make qemu           Boot the ISO interactively in QEMU' \
	  '  make test-qemu      Run the serial-console boot smoke test' \
	  '  make usb USB=/dev/sdX  Write ISO to USB (erases that drive)' \
	  '  make clean          Remove generated output' \
	  '' \
	  'Options: ARCH=x86_64|arm64 MINTOSH_HARDWARE_PROFILE=generic|hp-probook-450-g6 JOBS=<parallel jobs>' \
	  'The first live ISO and installer target x86_64.'
check-release:
	@python3 scripts/check-release.py
	@python3 -m json.tool configs/release.json >/dev/null

check-makefiles:
	@$(MAKE) --no-print-directory -n help >/dev/null
	@$(MAKE) --no-print-directory -C Sources/MintoshCore -n all test-foundation >/dev/null
	@echo 'Root and native-core Makefiles parse and resolve all build prerequisites.'

check-scripts:
	@python3 -c 'import pathlib, subprocess; bash={"mintosh-installer.sh", "installer-disk-safety.sh", "installer-storage.sh", "test-installer-disk-safety.sh", "test-installer-partitioning.sh"}; [subprocess.run(["bash" if p.name in bash else "sh", "-n", str(p)], check=True) for p in pathlib.Path("scripts").glob("*.sh")]'
	@python3 -c 'import ast,pathlib; ast.parse(pathlib.Path("scripts/check-release.py").read_text(encoding="utf-8"))'
	@sh -n scripts/configure-hp-probook.sh
	@echo 'Shell and Python scripts are syntactically valid.'

test-native-core:
	@sh Sources/MintoshCore/tests/smoke.sh

test-kernel-config:
	@sh scripts/test-kernel-config.sh

test-installer-safety:
	@bash scripts/test-installer-disk-safety.sh
	@bash scripts/test-installer-partitioning.sh

test-mboot: mboot
	@python3 scripts/test-mboot.py $(MBOOT_OUT)/BOOTX64.EFI

mboot: $(MBOOT_OUT)/BOOTX64.EFI

$(MBOOT_OUT):
	@mkdir -p $@

$(MBOOT_OUT)/mboot.o: Sources/mBoot/mboot.c | $(MBOOT_OUT)
	@test -n "$(EFI_CRT)" -a -n "$(EFI_LDS)" || { echo 'GNU-EFI development files are missing; install gnu-efi.' >&2; exit 1; }
	$(CC) -I/usr/include/efi -I/usr/include/efi/x86_64 -DEFI_FUNCTION_WRAPPER -O2 -Wall -Wextra -Werror -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -c $< -o $@

$(MBOOT_OUT)/mboot.so: $(MBOOT_OUT)/mboot.o
	ld -nostdlib -znocombreloc -T $(EFI_LDS) -shared -Bsymbolic $(EFI_CRT) $< -L$(EFI_LIBDIR) -lefi -lgnuefi -o $@

$(MBOOT_OUT)/BOOTX64.EFI: $(MBOOT_OUT)/mboot.so
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel -j .rela -j .reloc --target=efi-app-x86_64 $< $@

verify: check-release check-makefiles check-scripts test-kernel-config test-installer-safety test-native-core test-mboot
	@echo 'Mintosh validation and native core tests passed.'

release-info: check-release
	@python3 -c 'import json; d=json.load(open("configs/release.json")); print("Mintosh", d["release"]); print("Linux", d["kernel"]["version"]); print("Debian", d["debian_suite"]); print("Bootable architectures:", ", ".join(d["architectures"])); print("Kernel build architectures:", ", ".join(d["kernel_build_architectures"])); print("Planned architectures:", ", ".join(d["planned_architectures"]) or "none"); print("Bootable ISO:", d["image"]["primary_architecture"])'

check-host:
	@command -v make >/dev/null || { echo 'Missing dependency: make' >&2; exit 1; }
	@command -v python3 >/dev/null || { echo 'Missing dependency: python3' >&2; exit 1; }
	@command -v curl >/dev/null || { echo 'Missing dependency: curl' >&2; exit 1; }
	@command -v sha256sum >/dev/null || { echo 'Missing dependency: sha256sum' >&2; exit 1; }
	@case "$(ARCH)" in x86_64|arm64) ;; *) echo 'Unsupported ARCH: $(ARCH)' >&2; exit 1 ;; esac

host-deps:
	@sudo apt-get update
	@sudo apt-get install -y bc bison build-essential cpio curl debootstrap flex gcc g++ \
	  grub-common grub-pc-bin grub-efi-amd64-bin gnu-efi kmod libelf-dev libncurses-dev \
	  libssl-dev libsqlite3-dev make mtools parted python3 qemu-system-x86 util-linux rsync zip gcc-aarch64-linux-gnu binutils-aarch64-linux-gnu xorriso xz-utils

kernel-fetch: check-host check-release
	@ARCH="$(ARCH)" sh ./scripts/fetch-kernel.sh

kernel-config: check-host
	@test -d "$(KERNEL_DIR)" || { echo 'Run make kernel-fetch first.' >&2; exit 1; }
	@mkdir -p "$(KERNEL_OUT)"
	$(MAKE) -C "$(KERNEL_DIR)" O="$(CURDIR)/$(KERNEL_OUT)" ARCH="$(KERNEL_ARCH)" CROSS_COMPILE="$(CROSS_COMPILE)" $(KERNEL_DEFCONFIG)
	@"$(KERNEL_DIR)/scripts/config" --file "$(KERNEL_OUT)/.config" \
	  --enable MODULES --enable MODULE_SIG --enable MODULE_SIG_ALL --enable MODULE_SIG_FORCE --enable MODULE_SIG_SHA256 --enable BLK_DEV_INITRD --enable DEVTMPFS --enable DEVTMPFS_MOUNT \
	  --enable EXT4_FS --enable VIRTIO --enable VIRTIO_PCI --enable VIRTIO_BLK \
	  --enable VIRTIO_NET --enable SERIAL_8250 --enable SERIAL_8250_CONSOLE \
	  --enable EFI --enable EFI_STUB --enable EFI_PARTITION --enable EFI_VARS --enable FAT_FS --enable VFAT_FS --enable MSDOS_PARTITION \
	  --enable PROC_FS --enable SYSFS --enable TMPFS --enable TMPFS_POSIX_ACL \
	  --enable CGROUPS --enable INOTIFY_USER --enable SIGNALFD --enable TIMERFD \
	  --enable EPOLL --enable FHANDLE --enable AUTOFS4_FS --enable UNIX98_PTYS \
	  --enable PACKET --enable UNIX --enable INET --enable IPV6 --enable NET_NS \
	  --enable SECCOMP --enable SECCOMP_FILTER --set-str LOCALVERSION -mintosh
	$(MAKE) -C "$(KERNEL_DIR)" O="$(CURDIR)/$(KERNEL_OUT)" ARCH="$(KERNEL_ARCH)" CROSS_COMPILE="$(CROSS_COMPILE)" olddefconfig
	@case "$(MINTOSH_HARDWARE_PROFILE)" in generic|hp-probook|hp-probook-450-g6) ;; *) echo 'Unsupported MINTOSH_HARDWARE_PROFILE: $(MINTOSH_HARDWARE_PROFILE)' >&2; exit 1 ;; esac
	@if [ "$(ARCH)" = x86_64 ]; then \
	  sh scripts/configure-x86_64-hardware.sh "$(KERNEL_DIR)" "$(KERNEL_OUT)"; \
	  if [ "$(MINTOSH_HARDWARE_PROFILE)" = hp-probook ] || [ "$(MINTOSH_HARDWARE_PROFILE)" = hp-probook-450-g6 ]; then sh scripts/configure-hp-probook.sh "$(KERNEL_DIR)" "$(KERNEL_OUT)"; fi; \
	elif [ "$(MINTOSH_HARDWARE_PROFILE)" != generic ]; then \
	  echo 'The HP ProBook profile is only available for x86_64.' >&2; exit 1; \
	fi
	@sh scripts/verify-kernel-config.sh "$(KERNEL_OUT)/.config" "$(MINTOSH_HARDWARE_PROFILE)" "$(ARCH)"

kernel: check-host
	@test -f "$(KERNEL_OUT)/.config" || { echo 'Run make kernel-config first.' >&2; exit 1; }
	@sh scripts/verify-kernel-config.sh "$(KERNEL_OUT)/.config" "$(MINTOSH_HARDWARE_PROFILE)" "$(ARCH)"
	$(MAKE) -C "$(KERNEL_DIR)" O="$(CURDIR)/$(KERNEL_OUT)" ARCH="$(KERNEL_ARCH)" CROSS_COMPILE="$(CROSS_COMPILE)" -j"$(JOBS)" $(KERNEL_IMAGE) modules
	@echo 'Kernel build completed: $(KERNEL_OUT)'

kernel-install: rootfs kernel
	@test "$(ARCH)" = x86_64 || { echo 'Rootfs/ISO packaging currently targets x86_64.' >&2; exit 1; }
	@sudo mkdir -p "$(ROOTFS)/lib/modules" "$(ROOTFS)/boot"
	@sudo $(MAKE) -C "$(KERNEL_DIR)" O="$(CURDIR)/$(KERNEL_OUT)" ARCH="$(KERNEL_ARCH)" INSTALL_MOD_PATH="$(CURDIR)/$(ROOTFS)" modules_install
	@KREL=$$($(MAKE) -s -C "$(KERNEL_DIR)" O="$(CURDIR)/$(KERNEL_OUT)" ARCH="$(KERNEL_ARCH)" CROSS_COMPILE="$(CROSS_COMPILE)" kernelrelease); \
	  sudo install -m 0644 "$(KERNEL_OUT)/arch/x86/boot/bzImage" "$(ROOTFS)/boot/vmlinuz-$$KREL"

rootfs: check-host check-release
	@test "$(ARCH)" = x86_64 || { echo 'Rootfs/ISO packaging currently targets x86_64.' >&2; exit 1; }
	@sudo --preserve-env=ARCH,MINTOSH_USER,MINTOSH_PASSWORD sh ./scripts/build-rootfs.sh

initramfs: rootfs kernel-install
	@sudo sh ./scripts/build-initramfs.sh

iso: initramfs
	@test "$(ARCH)" = x86_64 || { echo 'Bootable ISO packaging currently targets x86_64.' >&2; exit 1; }
	@sudo ARCH="$(ARCH)" sh ./scripts/build-iso.sh

image: iso

qemu: iso
	qemu-system-x86_64 -m $(QEMU_MEMORY) -smp 2 -netdev user,id=net0 -device virtio-net-pci,netdev=net0 -cdrom "$(ISO)" -boot d

test-qemu: iso
	@QEMU_MEMORY="$(QEMU_MEMORY)" sh ./scripts/test-qemu.sh "$(ISO)"

usb: iso
	@test -n "$(USB)" || { echo 'Specify USB=/dev/sdX' >&2; exit 1; }
	sudo sh ./scripts/write-usb.sh "$(ISO)" "$(USB)"

clean:
	@sudo rm -rf out kernel/linux kernel/LINUX_VERSION
	@echo 'Removed Mintosh build output and downloaded kernel source.'

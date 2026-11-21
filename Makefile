SHELL := /bin/sh
ARCH ?= x86_64
JOBS ?= $(shell getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)
KERNEL_DIR := kernel/linux
KERNEL_OUT := out/kernel

.PHONY: help check-host kernel-fetch kernel-config kernel kernel-clean

help:
	@printf '%s\n' \
	  'Mintosh build targets:' \
	  '  make kernel-fetch   Download the latest stable Linux kernel source' \
	  '  make kernel-config  Generate a default kernel configuration' \
	  '  make kernel         Build the kernel' \
	  '  make kernel-clean   Remove kernel build output' \
	  '' \
	  'Options: ARCH=x86_64|arm64 JOBS=<parallel jobs>'

check-host:
	@command -v make >/dev/null || { echo 'Missing dependency: make' >&2; exit 1; }
	@command -v python3 >/dev/null || { echo 'Missing dependency: python3' >&2; exit 1; }
	@command -v curl >/dev/null || { echo 'Missing dependency: curl' >&2; exit 1; }
	@case "$(ARCH)" in x86_64|arm64) ;; *) echo 'Unsupported ARCH: $(ARCH)' >&2; exit 1 ;; esac

kernel-fetch: check-host
	@ARCH="$(ARCH)" ./scripts/fetch-kernel.sh

kernel-config:
	@test -d "$(KERNEL_DIR)" || { echo 'Run make kernel-fetch first.' >&2; exit 1; }
	@mkdir -p "$(KERNEL_OUT)"
	$(MAKE) -C "$(KERNEL_DIR)" O="$(CURDIR)/$(KERNEL_OUT)" ARCH="$(ARCH)" defconfig

kernel:
	@test -f "$(KERNEL_OUT)/.config" || { echo 'Run make kernel-config first.' >&2; exit 1; }
	$(MAKE) -C "$(KERNEL_DIR)" O="$(CURDIR)/$(KERNEL_OUT)" ARCH="$(ARCH)" -j"$(JOBS)"
	@echo 'Kernel build completed. Image and modules are in $(KERNEL_OUT).'

kernel-clean:
	@rm -rf "$(KERNEL_OUT)"

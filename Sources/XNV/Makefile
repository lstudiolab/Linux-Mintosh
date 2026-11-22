CC ?= cc
CFLAGS ?= -O2 -std=c17 -Wall -Wextra -Wpedantic -Werror
BUILD := build
EFI_CRT ?= $(firstword $(wildcard /usr/lib/crt0-efi-x86_64.o /usr/lib/gnuefi/crt0-efi-x86_64.o))
EFI_LDS ?= $(firstword $(wildcard /usr/lib/elf_x86_64_efi.lds /usr/lib/gnuefi/elf_x86_64_efi.lds))
EFI_LIBDIR ?= $(dir $(EFI_CRT))
.PHONY: all verify clean
all: $(BUILD)/boot-contract.o $(BUILD)/memory.o $(BUILD)/paging.o $(BUILD)/interrupts.o $(BUILD)/serial.o $(BUILD)/entry.o $(BUILD)/isr.o
$(BUILD):
	mkdir -p $@
$(BUILD)/boot-contract.o: boot_contract.c include/xnv/boot.h | $(BUILD)
	$(CC) $(CFLAGS) -ffreestanding -fno-builtin -fno-stack-protector -mno-red-zone -I. -c $< -o $@
$(BUILD)/memory.o: memory.c include/xnv/memory.h | $(BUILD)
	$(CC) $(CFLAGS) -ffreestanding -fno-builtin -fno-stack-protector -mno-red-zone -I. -c $< -o $@
$(BUILD)/paging.o: paging.c include/xnv/arch.h | $(BUILD)
	$(CC) $(CFLAGS) -ffreestanding -fno-builtin -fno-stack-protector -mno-red-zone -I. -c $< -o $@
$(BUILD)/interrupts.o: interrupts.c include/xnv/arch.h | $(BUILD)
	$(CC) $(CFLAGS) -ffreestanding -fno-builtin -fno-stack-protector -mno-red-zone -I. -c $< -o $@
$(BUILD)/serial.o: serial.c include/xnv/arch.h | $(BUILD)
	$(CC) $(CFLAGS) -ffreestanding -fno-builtin -fno-stack-protector -mno-red-zone -I. -c $< -o $@
$(BUILD)/entry.o: arch/x86_64/entry.S | $(BUILD)
	$(CC) -c -ffreestanding -fno-pic -mno-red-zone $< -o $@
$(BUILD)/isr.o: arch/x86_64/interrupts.S | $(BUILD)
	$(CC) -c -ffreestanding -fno-pic -mno-red-zone $< -o $@
$(BUILD)/efi_main.o: efi_main.c include/xnv/boot.h include/xnv/arch.h | $(BUILD)
	@test -n "$(EFI_CRT)" -a -n "$(EFI_LDS)" || { echo 'GNU-EFI development files are missing; install gnu-efi.' >&2; exit 1; }
	$(CC) -I/usr/include/efi -I/usr/include/efi/x86_64 -I. -DEFI_FUNCTION_WRAPPER -DGNU_EFI_USE_MS_ABI -O2 -Wall -Wextra -Werror -fpic -fno-strict-aliasing -fno-merge-constants -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -c $< -o $@
$(BUILD)/xnv.so: $(BUILD)/efi_main.o $(BUILD)/entry.o $(BUILD)/isr.o $(BUILD)/boot-contract.o $(BUILD)/memory.o $(BUILD)/paging.o $(BUILD)/interrupts.o $(BUILD)/serial.o
	ld -nostdlib -znocombreloc -T $(EFI_LDS) -shared -Bsymbolic $(EFI_CRT) $^ -L$(EFI_LIBDIR) -lefi -lgnuefi -o $@
$(BUILD)/XNV.EFI: $(BUILD)/xnv.so
	objcopy -j .peheader -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym -j .rel -j .rela -j .rel.* -j .rela.* -j .bss -j .reloc --target=efi-app-x86_64 --subsystem=10 $< $@
verify: all $(BUILD)/boot-contract-tests $(BUILD)/memory-tests
	$(BUILD)/boot-contract-tests
	$(BUILD)/memory-tests
	@echo 'XNV freestanding core and host regression tests passed.'
$(BUILD)/boot-contract-tests: tests/boot_contract_tests.c boot_contract.c include/xnv/boot.h | $(BUILD)
	$(CC) $(CFLAGS) -I. $^ -o $@
$(BUILD)/memory-tests: tests/memory_tests.c memory.c include/xnv/memory.h | $(BUILD)
	$(CC) $(CFLAGS) -I. $^ -o $@
clean:
	rm -rf $(BUILD)

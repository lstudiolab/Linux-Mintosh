CC ?= cc
CFLAGS ?= -O2 -std=c17 -Wall -Wextra -Wpedantic -Werror
BUILD := build

.PHONY: all verify clean
all: $(BUILD)/boot-contract.o

$(BUILD):
	mkdir -p $@

$(BUILD)/boot-contract.o: boot_contract.c include/xnv/boot.h | $(BUILD)
	$(CC) $(CFLAGS) -ffreestanding -fno-builtin -fno-stack-protector -mno-red-zone -I. -c $< -o $@

$(BUILD)/boot-contract-tests: tests/boot_contract_tests.c boot_contract.c include/xnv/boot.h | $(BUILD)
	$(CC) $(CFLAGS) -I. tests/boot_contract_tests.c boot_contract.c -o $@

verify: all $(BUILD)/boot-contract-tests
	$(BUILD)/boot-contract-tests
	@echo 'XNV boot contract compiles freestanding and passes host regression tests.'

clean:
	rm -rf $(BUILD)

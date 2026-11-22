CC ?= cc
CFLAGS ?= -std=c17 -O2 -Wall -Wextra -Wpedantic -Wformat=2 -Werror -fstack-protector-strong -D_FORTIFY_SOURCE=2
PREFIX ?= /usr
DESTDIR ?=
BINDIR = $(DESTDIR)$(PREFIX)/bin
BUILD = build

.PHONY: all test install clean
all: $(BUILD)/sean-system

$(BUILD):
	mkdir -p $@

$(BUILD)/sean-system: system_service.c include/sean/protocol.h | $(BUILD)
	$(CC) $(CFLAGS) -I. system_service.c -o $@

test: $(BUILD)/sean-system
	./$(BUILD)/sean-system --version

install: all
	install -d $(BINDIR)
	install -m 0755 $(BUILD)/sean-system $(BINDIR)/sean-system

clean:
	rm -rf $(BUILD)

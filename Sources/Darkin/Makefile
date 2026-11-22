CC ?= cc
CFLAGS ?= -std=c17 -O2 -Wall -Wextra -Wpedantic -Wformat=2 -Werror -fstack-protector-strong -D_FORTIFY_SOURCE=2
PREFIX ?= /usr
DESTDIR ?=
BINDIR = $(DESTDIR)$(PREFIX)/bin
BUILD = build

.PHONY: all test install clean
all: $(BUILD)/darkin-system

$(BUILD):
	mkdir -p $@

$(BUILD)/darkin-system: system_service.c include/darkin/protocol.h | $(BUILD)
	$(CC) $(CFLAGS) -I. system_service.c -o $@

test: $(BUILD)/darkin-system
	./$(BUILD)/darkin-system --version

install: all
	install -d $(BINDIR)
	install -m 0755 $(BUILD)/darkin-system $(BINDIR)/darkin-system

clean:
	rm -rf $(BUILD)

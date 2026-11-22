CC ?= cc
AR ?= ar
CFLAGS ?= -std=c17 -O2 -Wall -Wextra -Wpedantic -Wformat=2 -Werror -fstack-protector-strong -D_FORTIFY_SOURCE=2
PREFIX ?= /usr
DESTDIR ?=
BINDIR = $(DESTDIR)$(PREFIX)/bin
LIBDIR = $(DESTDIR)$(PREFIX)/lib
INCLUDEDIR = $(DESTDIR)$(PREFIX)/include/xcm
BUILD = build

.PHONY: all test install clean
all: $(BUILD)/xcmd $(BUILD)/libxcm.a

$(BUILD):
	mkdir -p $@

$(BUILD)/policy.o: policy.c include/xcm/protocol.h | $(BUILD)
	$(CC) $(CFLAGS) -I. -c $< -o $@

$(BUILD)/client.o: client.c include/xcm/client.h include/xcm/protocol.h | $(BUILD)
	$(CC) $(CFLAGS) -I. -c $< -o $@

$(BUILD)/xcmd: xcmd.c policy.c darkin_client.c include/xcm/protocol.h include/xcm/darkin_client.h ../Darkin/include/darkin/protocol.h | $(BUILD)
	$(CC) $(CFLAGS) -I. -I../Darkin/include xcmd.c policy.c darkin_client.c -o $@

$(BUILD)/libxcm.a: $(BUILD)/client.o $(BUILD)/policy.o
	$(AR) rcs $@ $^

$(BUILD)/policy-tests: tests/policy_tests.c policy.c include/xcm/protocol.h | $(BUILD)
	$(CC) $(CFLAGS) -I. tests/policy_tests.c policy.c -o $@

$(BUILD)/client-tests: tests/client_tests.c client.c include/xcm/client.h include/xcm/protocol.h | $(BUILD)
	$(CC) $(CFLAGS) -I. tests/client_tests.c client.c -o $@

test: $(BUILD)/policy-tests $(BUILD)/client-tests $(BUILD)/xcmd
	./$(BUILD)/policy-tests
	./$(BUILD)/client-tests
	./$(BUILD)/xcmd --version

install: all
	install -d $(BINDIR) $(LIBDIR) $(INCLUDEDIR)
	install -m 0755 $(BUILD)/xcmd $(BINDIR)/xcmd
	install -m 0644 $(BUILD)/libxcm.a $(LIBDIR)/libxcm.a
	install -m 0644 include/xcm/protocol.h include/xcm/client.h $(INCLUDEDIR)/

clean:
	rm -rf $(BUILD)

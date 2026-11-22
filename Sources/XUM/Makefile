CC ?= cc
AR ?= ar
CFLAGS ?= -std=c17 -O2 -Wall -Wextra -Wpedantic -Wformat=2 -Werror -fstack-protector-strong -D_FORTIFY_SOURCE=2
PREFIX ?= /usr
DESTDIR ?=
BINDIR = $(DESTDIR)$(PREFIX)/bin
LIBDIR = $(DESTDIR)$(PREFIX)/lib
INCLUDEDIR = $(DESTDIR)$(PREFIX)/include/xcm
XPU_INCLUDEDIR = $(DESTDIR)$(PREFIX)/include/xpu
BUILD = build

.PHONY: all test install clean
all: $(BUILD)/xpu $(BUILD)/libxcm.a $(BUILD)/libxpu.a

$(BUILD):
	mkdir -p $@

$(BUILD)/policy.o: policy.c include/xcm/protocol.h | $(BUILD)
	$(CC) $(CFLAGS) -I. -c $< -o $@

$(BUILD)/client.o: client.c include/xcm/client.h include/xcm/protocol.h | $(BUILD)
	$(CC) $(CFLAGS) -I. -c $< -o $@

$(BUILD)/xpu: xcmd.c policy.c sean_client.c usi_token_validation.c include/xcm/protocol.h include/xcm/sean_client.h include/xcm/usi_tokens.h ../Sean/include/sean/protocol.h | $(BUILD)
	$(CC) $(CFLAGS) -I. -I../Sean/include xcmd.c policy.c sean_client.c usi_token_validation.c -o $@

$(BUILD)/libxcm.a: $(BUILD)/client.o $(BUILD)/policy.o
	$(AR) rcs $@ $^

$(BUILD)/libxpu.a: $(BUILD)/libxcm.a
	cp $< $@

$(BUILD)/policy-tests: tests/policy_tests.c policy.c include/xcm/protocol.h | $(BUILD)
	$(CC) $(CFLAGS) -I. tests/policy_tests.c policy.c -o $@

$(BUILD)/client-tests: tests/client_tests.c client.c include/xcm/client.h include/xcm/protocol.h | $(BUILD)
	$(CC) $(CFLAGS) -I. tests/client_tests.c client.c -o $@

$(BUILD)/usi-token-tests: tests/usi_token_tests.c usi_token_validation.c include/xcm/usi_tokens.h include/xcm/protocol.h | $(BUILD)
	$(CC) $(CFLAGS) -I. tests/usi_token_tests.c usi_token_validation.c -o $@

$(BUILD)/xpu-api-tests: tests/xpu_api_tests.c include/xpu/xpu.h include/xcm/client.h include/xcm/protocol.h | $(BUILD)
	$(CC) $(CFLAGS) -Iinclude tests/xpu_api_tests.c -o $@

test: all $(BUILD)/policy-tests $(BUILD)/client-tests $(BUILD)/usi-token-tests $(BUILD)/xpu-api-tests $(BUILD)/xpu
	./$(BUILD)/policy-tests
	./$(BUILD)/client-tests
	./$(BUILD)/usi-token-tests
	./$(BUILD)/xpu-api-tests
	./$(BUILD)/xpu --version

install: all
	install -d $(BINDIR) $(LIBDIR) $(INCLUDEDIR) $(XPU_INCLUDEDIR)
	install -m 0755 $(BUILD)/xpu $(BINDIR)/xpu
	install -m 0644 $(BUILD)/libxcm.a $(LIBDIR)/libxcm.a
	install -m 0644 $(BUILD)/libxpu.a $(LIBDIR)/libxpu.a
	install -m 0644 include/xcm/protocol.h include/xcm/client.h include/xcm/usi_tokens.h $(INCLUDEDIR)/
	install -m 0644 include/xpu/xpu.h $(XPU_INCLUDEDIR)/xpu.h

clean:
	rm -rf $(BUILD)

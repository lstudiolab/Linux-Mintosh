CC ?= cc
AR ?= ar
CFLAGS ?= -std=c17 -O2 -Wall -Wextra -Wpedantic -Wformat=2 -Werror -fstack-protector-strong -D_FORTIFY_SOURCE=2
PREFIX ?= /usr
DESTDIR ?=
LIBDIR = $(DESTDIR)$(PREFIX)/lib
INCLUDEDIR = $(DESTDIR)$(PREFIX)/include/xnv
XCU_INCLUDEDIR = $(DESTDIR)$(PREFIX)/include/xcu
BUILD = build
SOURCES = task.c port.c memory.c event.c
OBJECTS = $(patsubst %.c,$(BUILD)/%.o,$(SOURCES))

.PHONY: all test install clean
all: $(BUILD)/libxcu.a $(BUILD)/libxnv.a

$(BUILD):
	mkdir -p $@

$(BUILD)/%.o: %.c include/xnv/xnv.h | $(BUILD)
	$(CC) $(CFLAGS) -I. -c $< -o $@

$(BUILD)/libxcu.a: $(OBJECTS)
	$(AR) rcs $@ $^

$(BUILD)/libxnv.a: $(BUILD)/libxcu.a
	cp $< $@

$(BUILD)/tests: tests/xnv_tests.c $(SOURCES) include/xnv/xnv.h | $(BUILD)
	$(CC) $(CFLAGS) -I. tests/xnv_tests.c $(SOURCES) -o $@

$(BUILD)/xcu-api-tests: tests/xcu_api_tests.c $(SOURCES) include/xnv/xnv.h include/xcu/xcu.h | $(BUILD)
	$(CC) $(CFLAGS) -I. tests/xcu_api_tests.c $(SOURCES) -o $@

test: $(BUILD)/tests $(BUILD)/xcu-api-tests
	./$(BUILD)/tests
	./$(BUILD)/xcu-api-tests

install: all
	install -d $(LIBDIR) $(INCLUDEDIR) $(XCU_INCLUDEDIR)
	install -m 0644 $(BUILD)/libxcu.a $(LIBDIR)/libxcu.a
	install -m 0644 $(BUILD)/libxnv.a $(LIBDIR)/libxnv.a
	install -m 0644 include/xnv/xnv.h $(INCLUDEDIR)/xnv.h
	install -m 0644 include/xcu/xcu.h $(XCU_INCLUDEDIR)/xcu.h

clean:
	rm -rf $(BUILD)

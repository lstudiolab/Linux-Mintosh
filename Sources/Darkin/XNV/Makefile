CC ?= cc
AR ?= ar
CFLAGS ?= -std=c17 -O2 -Wall -Wextra -Wpedantic -Wformat=2 -Werror -fstack-protector-strong -D_FORTIFY_SOURCE=2
PREFIX ?= /usr
DESTDIR ?=
LIBDIR = $(DESTDIR)$(PREFIX)/lib
INCLUDEDIR = $(DESTDIR)$(PREFIX)/include/xnv
BUILD = build
SOURCES = task.c port.c memory.c event.c
OBJECTS = $(patsubst %.c,$(BUILD)/%.o,$(SOURCES))

.PHONY: all test install clean
all: $(BUILD)/libxnv.a

$(BUILD):
	mkdir -p $@

$(BUILD)/%.o: %.c include/xnv/xnv.h | $(BUILD)
	$(CC) $(CFLAGS) -I. -c $< -o $@

$(BUILD)/libxnv.a: $(OBJECTS)
	$(AR) rcs $@ $^

$(BUILD)/tests: tests/xnv_tests.c $(SOURCES) include/xnv/xnv.h | $(BUILD)
	$(CC) $(CFLAGS) -I. tests/xnv_tests.c $(SOURCES) -o $@

test: $(BUILD)/tests
	./$(BUILD)/tests

install: all
	install -d $(LIBDIR) $(INCLUDEDIR)
	install -m 0644 $(BUILD)/libxnv.a $(LIBDIR)/libxnv.a
	install -m 0644 include/xnv/xnv.h $(INCLUDEDIR)/xnv.h

clean:
	rm -rf $(BUILD)

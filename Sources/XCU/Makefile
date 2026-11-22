CC ?= cc
AR ?= ar
CFLAGS ?= -std=c17 -O2 -Wall -Wextra -Wpedantic -Wformat=2 -Werror -fstack-protector-strong -D_FORTIFY_SOURCE=2
PREFIX ?= /usr
DESTDIR ?=
BUILD := build
SOURCES := Primitives/task.c Primitives/port.c Primitives/memory.c Primitives/event.c
OBJECTS := $(patsubst Primitives/%.c,$(BUILD)/%.o,$(SOURCES))

.PHONY: all test install clean
all: $(BUILD)/libxcu.a

$(BUILD):
	mkdir -p $@

$(BUILD)/%.o: Primitives/%.c include/xcu/xcu.h | $(BUILD)
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@

$(BUILD)/libxcu.a: $(OBJECTS)
	$(AR) rcs $@ $^

$(BUILD)/xcu-tests: tests/xcu_tests.c $(SOURCES) include/xcu/xcu.h | $(BUILD)
	$(CC) $(CFLAGS) -Iinclude tests/xcu_tests.c $(SOURCES) -o $@

$(BUILD)/upstream-compat-tests: tests/upstream_compat_tests.c | $(BUILD)
	$(CC) $(CFLAGS) $< -o $@ -lrt

test: all $(BUILD)/xcu-tests $(BUILD)/upstream-compat-tests
	./$(BUILD)/xcu-tests
	./$(BUILD)/upstream-compat-tests

install: all
	install -d $(DESTDIR)$(PREFIX)/lib $(DESTDIR)$(PREFIX)/include/xcu
	install -m 0644 $(BUILD)/libxcu.a $(DESTDIR)$(PREFIX)/lib/libxcu.a
	install -m 0644 include/xcu/xcu.h $(DESTDIR)$(PREFIX)/include/xcu/xcu.h

clean:
	rm -rf $(BUILD)

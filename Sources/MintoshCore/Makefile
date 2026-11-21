CC ?= cc
AR ?= ar
CFLAGS ?= -O2 -Wall -Wextra
CPPFLAGS += -I../LibSys/include -I../CSF/include -I../Foundation/include
PREFIX ?= /usr
DESTDIR ?=
BINDIR = $(DESTDIR)$(PREFIX)/bin
LIBDIR = $(DESTDIR)$(PREFIX)/lib
INCLUDEDIR = $(DESTDIR)$(PREFIX)/include/mintosh
BUILD = build
LIBSYS = $(BUILD)/libsys.o
PROGRAMS = mosf launch7762 launch511 ioplus kextctl mosflog security-audit mintoshctl
all: $(addprefix $(BUILD)/,$(PROGRAMS)) $(BUILD)/libcsf.a $(BUILD)/libfoundation.a
$(BUILD):
	mkdir -p $@
$(LIBSYS): ../LibSys/libsys.c ../LibSys/include/libsys.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
$(BUILD)/mosf: ../MOSF/main.c $(LIBSYS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
$(BUILD)/launch7762: ../Launch7762/main.c $(LIBSYS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
$(BUILD)/launch511: ../Launch511/main.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@
$(BUILD)/ioplus: ../IOPlus/main.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@
$(BUILD)/kextctl: ../Kexts/main.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@
$(BUILD)/mosflog: ../SystemLogging/main.c $(LIBSYS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
$(BUILD)/security-audit: ../Security/main.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@
$(BUILD)/mintoshctl: ../MintoshCtl/main.c $(LIBSYS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
$(BUILD)/libcsf.a: ../CSF/csf.c $(LIBSYS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c ../CSF/csf.c -o $(BUILD)/csf.o
	$(AR) rcs $@ $(BUILD)/csf.o $(LIBSYS)
$(BUILD)/libfoundation.a: ../Foundation/Foundation.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $(BUILD)/foundation.o
	$(AR) rcs $@ $(BUILD)/foundation.o
install: all
	install -d $(BINDIR) $(LIBDIR) $(INCLUDEDIR)
	install -m 0755 $(addprefix $(BUILD)/,$(PROGRAMS)) $(BINDIR)/
	install -m 0644 $(BUILD)/libcsf.a $(BUILD)/libfoundation.a $(LIBDIR)/
	install -m 0644 ../LibSys/include/libsys.h ../CSF/include/csf.h ../Foundation/include/Foundation.h $(INCLUDEDIR)/
clean:
	rm -rf $(BUILD)
.PHONY: all install clean

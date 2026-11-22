MAKE ?= make
CC ?= cc
PREFIX ?= /usr
DESTDIR ?=
LIBDIR = $(DESTDIR)$(PREFIX)/lib
INCLUDEDIR = $(DESTDIR)$(PREFIX)/include/coreservices
.DEFAULT_GOAL := all
.PHONY: all test install clean

all:
	$(MAKE) -C ../MintoshCore all

test:
	$(MAKE) -C ../MintoshCore test-foundation
	$(CC) -std=c17 -Wall -Wextra -Wpedantic -Werror -Iinclude -fsyntax-only tests/api_compile_test.c

install:
	$(MAKE) -C ../MintoshCore install
	install -d $(LIBDIR) $(INCLUDEDIR)
	install -m 0644 ../MintoshCore/build/libcsf.a $(LIBDIR)/libcoreservices.a
	install -m 0644 include/coreservices/coreservices.h include/coreservices/csf.h $(INCLUDEDIR)/

clean:
	$(MAKE) -C ../MintoshCore clean

CC ?= cc
CFLAGS ?= -std=c17 -O2 -Wall -Wextra -Wpedantic -Werror
.PHONY: all test clean
all: build/policy.o
build:
	mkdir -p $@
build/policy.o: policy.c include/xcm/protocol.h | build
	$(CC) $(CFLAGS) -I. -c $< -o $@
build/policy-tests: tests/policy_tests.c policy.c include/xcm/protocol.h | build
	$(CC) $(CFLAGS) -I. tests/policy_tests.c policy.c -o $@
test: build/policy-tests
	./build/policy-tests
clean:
	rm -rf build

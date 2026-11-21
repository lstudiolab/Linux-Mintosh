CC ?= cc
AR ?= ar
CFLAGS ?= -O2 -std=c17 -Wall -Wextra -Wpedantic -Wformat=2 -Werror=implicit-function-declaration -fstack-protector-strong -D_FORTIFY_SOURCE=2
CPPFLAGS += -I../LibSys/include -I../CSF/include -I../Foundation/include -I../Launch7762
PREFIX ?= /usr
DESTDIR ?=
BINDIR = $(DESTDIR)$(PREFIX)/bin
LIBDIR = $(DESTDIR)$(PREFIX)/lib
INCLUDEDIR = $(DESTDIR)$(PREFIX)/include/mintosh
BUILD = build

LIBSYS_SOURCES = ../LibSys/filesystem.c ../LibSys/file_descriptor.c ../LibSys/unix_socket.c ../LibSys/line_reader.c ../LibSys/system_log.c ../LibSys/runtime_paths.c
LIBSYS_OBJECTS = $(patsubst ../LibSys/%.c,$(BUILD)/libsys-%.o,$(LIBSYS_SOURCES))
PROGRAMS = darl launch7762 launch-client launch511 ioplus hp-compat kextctl mintosh-log security-audit mintoshctl mstore

all: $(addprefix $(BUILD)/,$(PROGRAMS)) $(BUILD)/libcsf.a $(BUILD)/libfoundation.a

$(BUILD):
	mkdir -p $@

$(BUILD)/libsys-%.o: ../LibSys/%.c ../LibSys/include/libsys.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD)/darl: ../Darl/darl_daemon.c ../Darl/darl_protocol.c $(LIBSYS_OBJECTS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/launch7762: ../Launch7762/service_manager_main.c ../Launch7762/service_config.c ../Launch7762/service_process.c ../Launch7762/service_control.c ../Launch7762/service_socket.c $(LIBSYS_OBJECTS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/launch-client: ../LaunchClient/launch_client.c $(LIBSYS_OBJECTS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/launch511: ../Launch511/command_runner.c ../Launch511/command_arguments.c ../Launch511/command_exec.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/hp-compat: ../HPCompatibility/hp_compatibility.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/ioplus: ../IOPlus/hardware_inventory.c ../IOPlus/kernel_information.c ../IOPlus/sysfs_inventory.c ../IOPlus/memory_information.c ../IOPlus/json_inventory.c ../IOPlus/uevent_parser.c ../IOPlus/uevent_monitor.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/kextctl: ../Kexts/module_manager.c ../Kexts/module_command.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/mintosh-log: ../SystemLogging/log_command.c ../SystemLogging/log_writer.c $(LIBSYS_OBJECTS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/security-audit: ../Security/permission_audit.c ../Security/permission_check.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/mintoshctl: ../MintoshCtl/control_command.c ../MintoshCtl/darl_client.c ../MintoshCtl/service_client.c $(LIBSYS_OBJECTS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/mstore: ../PackageManager/package_command.c ../PackageManager/package_validation.c ../PackageManager/package_exec.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/libcsf.a: ../CSF/darl_request.c $(LIBSYS_OBJECTS) | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c ../CSF/darl_request.c -o $(BUILD)/csf-darl-request.o
	$(AR) rcs $@ $(BUILD)/csf-darl-request.o $(LIBSYS_OBJECTS)

$(BUILD)/libfoundation.a: ../Foundation/string.c ../Foundation/path.c ../Foundation/data.c ../Foundation/array.c ../Foundation/dictionary.c ../Foundation/url.c ../Foundation/error.c ../Foundation/filesystem.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c ../Foundation/string.c -o $(BUILD)/foundation-string.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -c ../Foundation/path.c -o $(BUILD)/foundation-path.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -c ../Foundation/data.c -o $(BUILD)/foundation-data.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -c ../Foundation/array.c -o $(BUILD)/foundation-array.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -c ../Foundation/dictionary.c -o $(BUILD)/foundation-dictionary.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -c ../Foundation/url.c -o $(BUILD)/foundation-url.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -c ../Foundation/error.c -o $(BUILD)/foundation-error.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -c ../Foundation/filesystem.c -o $(BUILD)/foundation-filesystem.o
	$(AR) rcs $@ $(BUILD)/foundation-string.o $(BUILD)/foundation-path.o $(BUILD)/foundation-data.o $(BUILD)/foundation-array.o $(BUILD)/foundation-dictionary.o $(BUILD)/foundation-url.o $(BUILD)/foundation-error.o $(BUILD)/foundation-filesystem.o

$(BUILD)/foundation-tests: ../Foundation/tests/foundation_tests.c ../Foundation/string.c ../Foundation/path.c ../Foundation/data.c ../Foundation/array.c ../Foundation/dictionary.c ../Foundation/url.c ../Foundation/error.c ../Foundation/filesystem.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/libsys-tests: ../LibSys/tests/line_reader_tests.c ../LibSys/line_reader.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/ioplus-uevent-tests: ../IOPlus/tests/uevent_tests.c ../IOPlus/uevent_parser.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -I../IOPlus $^ -o $@

$(BUILD)/darl-protocol-tests: ../Darl/tests/darl_protocol_tests.c ../Darl/darl_protocol.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -I../Darl $^ -o $@

$(BUILD)/libsys-filesystem-tests: ../LibSys/tests/filesystem_tests.c ../LibSys/filesystem.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/package-validation-tests: ../PackageManager/tests/package_validation_tests.c ../PackageManager/package_validation.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/security-tests: ../Security/tests/security_tests.c ../Security/permission_check.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -I../Security $^ -o $@

$(BUILD)/service-control-tests: ../Launch7762/tests/service_control_tests.c ../Launch7762/service_control.c ../LibSys/file_descriptor.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -I../Launch7762 $^ -o $@

test-foundation: $(BUILD)/foundation-tests $(BUILD)/package-validation-tests $(BUILD)/service-control-tests $(BUILD)/libsys-tests $(BUILD)/libsys-filesystem-tests $(BUILD)/darl-protocol-tests $(BUILD)/ioplus-uevent-tests $(BUILD)/security-tests
	$(BUILD)/foundation-tests
	$(BUILD)/package-validation-tests
	$(BUILD)/service-control-tests
	$(BUILD)/libsys-tests
	$(BUILD)/libsys-filesystem-tests
	$(BUILD)/darl-protocol-tests
	$(BUILD)/ioplus-uevent-tests
	$(BUILD)/security-tests
	@echo 'Foundation, package validation and service control tests passed.'

install: all
	install -d $(BINDIR) $(LIBDIR) $(INCLUDEDIR)
	install -m 0755 $(addprefix $(BUILD)/,$(PROGRAMS)) $(BINDIR)/
	install -m 0644 $(BUILD)/libcsf.a $(BUILD)/libfoundation.a $(LIBDIR)/
	install -m 0644 ../LibSys/include/libsys.h ../CSF/include/csf.h ../Foundation/include/Foundation.h $(INCLUDEDIR)/

clean:
	rm -rf $(BUILD)

.PHONY: all install clean test-foundation

CC ?= cc
CXX ?= c++
AR ?= ar
CXXFLAGS ?= -O2 -std=c++20 -Wall -Wextra -Wpedantic -Wformat=2 -Werror -fstack-protector-strong -D_FORTIFY_SOURCE=2
CFLAGS ?= -O2 -std=c17 -Wall -Wextra -Wpedantic -Wformat=2 -Werror=implicit-function-declaration -Werror=multichar -Werror=type-limits -fstack-protector-strong -D_FORTIFY_SOURCE=2
CPPFLAGS += -I../LibSys/include -I../CSF/include -I../Foundation/include -I../Launch7762
PREFIX ?= /usr
DESTDIR ?=
BINDIR = $(DESTDIR)$(PREFIX)/bin
LIBDIR = $(DESTDIR)$(PREFIX)/lib
INCLUDEDIR = $(DESTDIR)$(PREFIX)/include/mintosh
BUILD = build

LIBSYS_SOURCES = ../LibSys/filesystem.c ../LibSys/file_descriptor.c ../LibSys/unix_socket.c ../LibSys/line_reader.c ../LibSys/system_log.c ../LibSys/runtime_paths.c
LIBSYS_OBJECTS = $(patsubst ../LibSys/%.c,$(BUILD)/libsys-%.o,$(LIBSYS_SOURCES))
PROGRAMS = mintosh-ready darl launch7762 launch-client launch511 ioplus iopsctl gvdctl fmsctl dmctl hp-compat kextctl mintosh-log security-audit mintoshctl mstore

all: $(addprefix $(BUILD)/,$(PROGRAMS)) $(BUILD)/libcsf.a $(BUILD)/libfoundation.a $(BUILD)/libgvd.a

$(BUILD):
	mkdir -p $@

$(BUILD)/libsys-%.o: ../LibSys/%.c ../LibSys/include/libsys.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD)/mintosh-ready: ../MintoshReady/mintosh_ready.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/darl: ../Darl/darl_daemon.c ../Darl/darl_protocol.c $(LIBSYS_OBJECTS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/launch7762: ../Launch7762/service_manager_main.c ../Launch7762/service_config.c ../Launch7762/service_process.c ../Launch7762/service_control.c ../Launch7762/service_socket.c $(LIBSYS_OBJECTS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/launch-client: ../LaunchClient/launch_client.c $(LIBSYS_OBJECTS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/launch511: ../Launch511/command_runner.c ../Launch511/command_arguments.c ../Launch511/command_exec.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/hp-compat: ../HPCompatibility/hp_compatibility.cpp ../Architecture/x86_64/cpu_identification.S | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@

$(BUILD)/gvdctl: ../GVD/task_manager.c ../GVD/task_policy.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -I../GVD $^ -o $@

$(BUILD)/iopsctl: ../IOPS/power_manager.c ../IOPS/power_control.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -I../IOPS $^ -o $@

$(BUILD)/fmsctl: ../FMS/fmsctl.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -I../FMS $^ -lsqlite3 -o $@

$(BUILD)/dmctl: ../DM/dmctl.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/ioplus: ../IOPlus/hardware_inventory.c ../IOPlus/kernel_information.c ../IOPlus/sysfs_inventory.c ../IOPlus/memory_information.c ../IOPlus/json_inventory.c ../IOPlus/uevent_parser.c ../IOPlus/uevent_monitor.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/kextctl: ../Kexts/module_manager.c ../Kexts/module_command.c ../Kexts/module_policy.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/mintosh-log: ../SystemLogging/log_command.c ../SystemLogging/log_writer.c $(LIBSYS_OBJECTS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/security-audit: ../Security/permission_audit.c ../Security/permission_check.c ../Kexts/module_policy.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -I../Kexts $^ -o $@

$(BUILD)/mintoshctl: ../MintoshCtl/control_command.c ../MintoshCtl/darl_client.c ../MintoshCtl/service_client.c $(LIBSYS_OBJECTS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/mstore: ../PackageManager/package_command.c ../PackageManager/package_validation.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/libcsf.a: ../CSF/darl_request.c ../CSF/system_info.c ../CSF/service_api.c $(LIBSYS_OBJECTS) | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c ../CSF/darl_request.c -o $(BUILD)/csf-darl-request.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -c ../CSF/system_info.c -o $(BUILD)/csf-system-info.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -c ../CSF/service_api.c -o $(BUILD)/csf-service-api.o
	$(AR) rcs $@ $(BUILD)/csf-darl-request.o $(BUILD)/csf-system-info.o $(BUILD)/csf-service-api.o $(LIBSYS_OBJECTS)

$(BUILD)/libgvd.a: ../GVD/dispatch_queue.c ../GVD/include/GVD.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -pthread -I../GVD/include -c ../GVD/dispatch_queue.c -o $(BUILD)/gvd-dispatch-queue.o
	$(AR) rcs $@ $(BUILD)/gvd-dispatch-queue.o

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

$(BUILD)/csf-tests: ../CSF/tests/csf_tests.c ../CSF/service_api.c ../CSF/system_info.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/gvd-dispatch-tests: ../GVD/tests/gvd_tests.c ../GVD/dispatch_queue.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -pthread -I../GVD/include $^ -o $@

$(BUILD)/foundation-tests: ../Foundation/tests/foundation_tests.c ../Foundation/string.c ../Foundation/path.c ../Foundation/data.c ../Foundation/array.c ../Foundation/dictionary.c ../Foundation/url.c ../Foundation/error.c ../Foundation/filesystem.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/foundation-tests-sanitize: ../Foundation/tests/foundation_tests.c ../Foundation/string.c ../Foundation/path.c ../Foundation/data.c ../Foundation/array.c ../Foundation/dictionary.c ../Foundation/url.c ../Foundation/error.c ../Foundation/filesystem.c | $(BUILD)
	$(CC) $(CPPFLAGS) -O1 -g -std=c17 -Wall -Wextra -Wpedantic -Wformat=2 -Werror -fsanitize=address,undefined -fno-omit-frame-pointer $^ -o $@

$(BUILD)/libsys-tests: ../LibSys/tests/line_reader_tests.c ../LibSys/line_reader.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/gvd-tests: ../GVD/tests/task_policy_tests.c ../GVD/task_policy.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -I../GVD $^ -o $@

$(BUILD)/iops-tests: ../IOPS/tests/power_control_tests.c ../IOPS/power_control.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -I../IOPS $^ -o $@

$(BUILD)/ioplus-uevent-tests: ../IOPlus/tests/uevent_tests.c ../IOPlus/uevent_parser.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -I../IOPlus $^ -o $@

$(BUILD)/darl-protocol-tests: ../Darl/tests/darl_protocol_tests.c ../Darl/darl_protocol.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -I../Darl $^ -o $@

$(BUILD)/libsys-filesystem-tests: ../LibSys/tests/filesystem_tests.c ../LibSys/filesystem.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/package-validation-tests: ../PackageManager/tests/package_validation_tests.c ../PackageManager/package_validation.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/kext-policy-tests: ../Kexts/tests/module_policy_tests.c ../Kexts/module_policy.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -I../Kexts $^ -o $@

$(BUILD)/security-tests: ../Security/tests/security_tests.c ../Security/permission_check.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -I../Security $^ -o $@

$(BUILD)/service-control-tests: ../Launch7762/tests/service_control_tests.c ../Launch7762/service_control.c ../LibSys/file_descriptor.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -I../Launch7762 $^ -o $@

test-foundation: $(BUILD)/foundation-tests $(BUILD)/foundation-tests-sanitize $(BUILD)/package-validation-tests $(BUILD)/service-control-tests $(BUILD)/libsys-tests $(BUILD)/libsys-filesystem-tests $(BUILD)/darl-protocol-tests $(BUILD)/ioplus-uevent-tests $(BUILD)/gvd-tests $(BUILD)/iops-tests $(BUILD)/gvd-dispatch-tests $(BUILD)/csf-tests $(BUILD)/kext-policy-tests $(BUILD)/security-tests
	$(BUILD)/foundation-tests
	ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 $(BUILD)/foundation-tests-sanitize
	$(BUILD)/package-validation-tests
	$(BUILD)/service-control-tests
	$(BUILD)/libsys-tests
	$(BUILD)/libsys-filesystem-tests
	$(BUILD)/darl-protocol-tests
	$(BUILD)/ioplus-uevent-tests
	$(BUILD)/gvd-tests
	$(BUILD)/iops-tests
	$(BUILD)/gvd-dispatch-tests
	$(BUILD)/csf-tests
	$(BUILD)/kext-policy-tests
	$(BUILD)/security-tests
	@echo 'Foundation, package validation and service control tests passed.'

install: all
	install -d $(BINDIR) $(LIBDIR) $(INCLUDEDIR)
	install -m 0755 $(addprefix $(BUILD)/,$(PROGRAMS)) $(BINDIR)/
	install -m 0644 $(BUILD)/libcsf.a $(BUILD)/libfoundation.a $(BUILD)/libgvd.a $(LIBDIR)/
	install -m 0644 ../LibSys/include/libsys.h ../CSF/include/csf.h ../Foundation/include/Foundation.h ../GVD/include/GVD.h $(INCLUDEDIR)/

clean:
	rm -rf $(BUILD)

.PHONY: all install clean test-foundation

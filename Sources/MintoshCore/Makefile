CC ?= cc
AR ?= ar
CFLAGS ?= -O2 -Wall -Wextra -Wpedantic
CPPFLAGS += -I../LibSys/include -I../CSF/include -I../Foundation/include -I../Launch7762
PREFIX ?= /usr
DESTDIR ?=
BINDIR = $(DESTDIR)$(PREFIX)/bin
LIBDIR = $(DESTDIR)$(PREFIX)/lib
INCLUDEDIR = $(DESTDIR)$(PREFIX)/include/mintosh
BUILD = build

LIBSYS_SOURCES = ../LibSys/filesystem.c ../LibSys/file_descriptor.c ../LibSys/unix_socket.c ../LibSys/system_log.c ../LibSys/runtime_paths.c
LIBSYS_OBJECTS = $(patsubst ../LibSys/%.c,$(BUILD)/libsys-%.o,$(LIBSYS_SOURCES))
PROGRAMS = darl launch7762 launch511 ioplus kextctl darllog security-audit mintoshctl mstore

all: $(addprefix $(BUILD)/,$(PROGRAMS)) $(BUILD)/libcsf.a $(BUILD)/libfoundation.a

$(BUILD):
	mkdir -p $@

$(BUILD)/libsys-%.o: ../LibSys/%.c ../LibSys/include/libsys.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD)/darl: ../Darl/darl_daemon.c ../Darl/darl_protocol.c $(LIBSYS_OBJECTS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/launch7762: ../Launch7762/service_manager_main.c ../Launch7762/service_config.c ../Launch7762/service_process.c ../Launch7762/service_control.c ../Launch7762/service_socket.c $(LIBSYS_OBJECTS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/launch511: ../Launch511/command_runner.c ../Launch511/command_arguments.c ../Launch511/command_exec.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/ioplus: ../IOPlus/hardware_inventory.c ../IOPlus/kernel_information.c ../IOPlus/sysfs_inventory.c ../IOPlus/memory_information.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/kextctl: ../Kexts/module_manager.c ../Kexts/module_command.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/darllog: ../SystemLogging/log_command.c ../SystemLogging/log_writer.c $(LIBSYS_OBJECTS)
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

$(BUILD)/libfoundation.a: ../Foundation/string.c ../Foundation/path.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c ../Foundation/string.c -o $(BUILD)/foundation-string.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -c ../Foundation/path.c -o $(BUILD)/foundation-path.o
	$(AR) rcs $@ $(BUILD)/foundation-string.o $(BUILD)/foundation-path.o

install: all
	install -d $(BINDIR) $(LIBDIR) $(INCLUDEDIR)
	install -m 0755 $(addprefix $(BUILD)/,$(PROGRAMS)) $(BINDIR)/
	install -m 0644 $(BUILD)/libcsf.a $(BUILD)/libfoundation.a $(LIBDIR)/
	install -m 0644 ../LibSys/include/libsys.h ../CSF/include/csf.h ../Foundation/include/Foundation.h $(INCLUDEDIR)/

clean:
	rm -rf $(BUILD)

.PHONY: all install clean

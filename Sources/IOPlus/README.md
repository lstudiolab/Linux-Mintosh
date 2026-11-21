# I/O+ hardware inventory

I/O+ reads Linux kernel, procfs and sysfs state. It does not replace kernel drivers.

## Commands

- `ioplus`: human-readable kernel, memory and device-class inventory.
- `ioplus --json`: structured kernel identity, CPU model, memory, firmware/DMI identity, and block, network, USB, graphics and power-supply device properties.
- `ioplus watch`: stream kernel uevents as newline-delimited JSON. Handles add, remove, change, bind, unbind, move and power-state device events. Stop with SIGINT or SIGTERM.

Hardware properties are read from sysfs at query time. Optional attributes are omitted when the kernel or device does not expose them. The watcher filters malformed, truncated and non-kernel netlink messages and reports locally dropped events.

## Scope

I/O+ is inventory and event transport. Driver loading remains the Linux kernel/kmod responsibility; policy and privileged operations belong to Security and Kexts.

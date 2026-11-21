# I/O+

I/O+ is Mintosh's read-only hardware and kernel inventory command.

- `ioplus`: human-readable kernel, memory and sysfs device summary.
- `ioplus --json`: machine-readable JSON with kernel identity, total/available memory and block, network, USB and graphics device names.
- `ioplus --version`: report the component version.

The initial inventory reads Linux `uname`, `/proc/meminfo` and sysfs class directories. It does not load drivers, change device state, or claim that every listed device has a working driver. Detailed capabilities, hotplug notifications, power state and driver-health reporting remain future work.

# Deprecated Tarts source root

Tarts was an earlier name for Lenix's system foundation. Its current and only canonical name is **Sean**.

The canonical backend source root is `Sources/Sean/`. New backend code, service implementations and public protocols belong there. The old Tarts directory is retained temporarily for build compatibility only.

Sean is the non-graphical foundation above XCU. It owns init, service lifecycle, hardware coordination, storage, networking, security, power, logging and recovery. Core Services exposes stable APIs to Choca and other clients. XUM provides authenticated cross-process messaging.

The current preview still uses transitional Debian userspace and systemd. Native Sean init and service management are not yet the default boot path.

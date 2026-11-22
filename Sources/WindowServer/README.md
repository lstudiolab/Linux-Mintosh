# Window Server

Window Server is a protected graphical-session service, separate from Cacao. It owns display discovery and modes, windows, surfaces, composition, stacking, focus, hit testing and input routing.

It accesses graphics and input devices through Linux/XCU interfaces, enforces per-session isolation, and treats all client surfaces and input requests as untrusted. Cacao uses a documented client protocol; applications never receive direct privileged display access.

The service is not yet implemented. This directory is the canonical source root for the Window Server.

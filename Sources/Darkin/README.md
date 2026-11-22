# Darkin — Mintosh system foundation

Darkin is the complete backend foundation. It owns XNV (the Linux-based kernel architecture), boot/runtime, hardware and device services, process and memory services, storage, filesystems, networking, security, logging, power and all non-graphical system services.

Darkin is independent of the graphical platform. It does not contain USI, a window server, graphical frameworks, desktop shell or app manager.

Existing backend prototypes in `Sources/` are Darkin-owned candidates. Their current Linux/POSIX dependencies and service boundaries must be audited before they are treated as final Darkin components.

Darkin exposes typed, versioned service APIs. It does not accept arbitrary code, pointers, paths or untrusted UI tokens as privileged commands. Choca reaches Darkin only through XCM. Each Darkin service must enforce authorization itself even when XCM has already checked the request.

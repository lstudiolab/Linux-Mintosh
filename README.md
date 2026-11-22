# Mintosh

Mintosh has three major system layers: **Darkin**, **XCM**, and **Choca**.

- **Darkin** is the complete backend foundation. It owns the kernel, boot/runtime infrastructure, hardware access, memory and process management, storage, networking, security enforcement, system services and backend APIs.
- **XNV** is the kernel architecture inside Darkin. Linux is the actual and primary kernel. First-party BSD-derived interfaces provide Unix/BSD compatibility, while Mach-inspired tasks, ports, message rights and IPC concepts provide a structured communication model. XNV is not a second kernel.
- **XCM** is a separate system communication and security broker outside Darkin. It mediates every Choca-to-Darkin request, validates message schemas, checks capabilities and protects the foundation from unauthorized changes.
- **Choca** is the graphical platform outside Darkin. It owns USI, the window server, graphical frameworks and the app manager. Choca produces typed UI-token streams; XCM validates and routes them to presentation services.

Dependency direction:

`Choca / USI → XCM → Darkin services → XNV → Linux kernel`

Darkin does not depend on Choca. It must boot and provide backend services without the graphical platform. XCM is the only supported bridge between Choca and Darkin.

Linux supplies the real kernel execution foundation: scheduling, virtual memory, drivers, VFS, networking, security hooks and hardware support. The BSD contribution is a first-party compatibility layer, not a second FreeBSD kernel. Mach-inspired IPC/task abstractions are implemented using Linux primitives. Mintosh does not copy Apple's XNU source.

## Current status

The Developer Preview boots upstream Linux with a transitional Debian userspace. The existing `Sources/XNV` UEFI code is an experimental standalone-kernel bootstrap and is **not** the selected XNV implementation; it is retained only as a reference prototype. Do not use its EFI image as the Mintosh kernel.

Darkin, XCM and Choca architecture contracts are being established before their full implementations. Existing backend prototypes under `Sources/` still need migration to final Darkin service contracts. Choca/USI graphical implementation remains deferred until the language and toolchain are ready.

## Build and verification

    make verify
    make kernel-fetch
    make kernel-config
    make kernel
    make iso
    make test-qemu

The current bootable preview is transitional, not the completed Darkin foundation or final Mintosh release.

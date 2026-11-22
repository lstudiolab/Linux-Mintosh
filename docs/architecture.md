# Mintosh system architecture

## Darkin — backend foundation

Darkin is the complete backend foundation and owns all privileged and non-graphical system responsibilities: XNV kernel integration, boot, CPU/platform setup, memory, process/thread management, devices, storage, filesystems, networking, security, logging, power, hardware management and system services.

Darkin must operate without Choca. It must not load UI code or accept direct filesystem/code mutation requests from the graphical platform.

## XNV — Linux-based kernel architecture inside Darkin

XNV uses a Linux-main-kernel model:

1. **Linux is the actual kernel.** Its scheduler, virtual memory, VFS, networking, device model, drivers, interrupt handling, security framework and hardware support are the execution foundation.
2. **BSD-derived functionality** supplies first-party compatibility interfaces: Unix/BSD-oriented process and socket semantics and selected BSD-style event/API behavior. This is implemented on Linux facilities; it is not a second BSD kernel.
3. **Mach-inspired functionality** supplies structured task/thread abstractions, IPC messages, port-like endpoints, capability rights and memory-object concepts. These are first-party abstractions backed by Linux primitives; they do not replace the Linux scheduler or memory manager.

XNV is the Mintosh kernel architecture and integration layer, not an independently booted kernel.

## XCM — external communication and security boundary

XCM is a separate broker between Choca and Darkin. It is not part of the kernel, Darkin or the graphical platform.

- Choca sends versioned, typed requests to XCM.
- XCM authenticates the local peer, validates message bounds/schema, checks operation-specific capabilities and applies policy before forwarding.
- Darkin services accept requests only through defined service interfaces; they never trust Choca pointers, arbitrary paths, executable code or raw UI tokens.
- Responses are typed data/events routed back through XCM.
- Unknown operations, malformed messages, privilege escalation, protected-foundation mutation and out-of-scope requests are rejected and audited.

Choca cannot directly modify or delete Darkin's kernel, boot files, system binaries, security policy or service implementation. Foundation updates require a separate authenticated, signed update/rollback workflow. User-data deletion is separate and requires an explicit capability, object-scoped target and confirmation.

XCM is one policy enforcement point, not a substitute for authorization inside each Darkin service and kernel boundary.

## Choca — graphical platform

Choca is separate from Darkin and contains USI, the window server/compositor, graphical frameworks, desktop shell, Finder-like UI, dock, launch UI, app manager and package/install UI.

Choca may emit typed UI-token streams for presentation. These tokens are data, not executable authority. XCM validates and routes them only to the intended rendering endpoint; they cannot become Darkin commands.

## Communication direction

`Choca / USI → XCM → Darkin service API → XNV → Linux`

Replies travel back through XCM. Darkin has no dependency on Choca.

## Migration status

The current preview boots upstream Linux and a transitional Debian userspace. The earlier standalone XNV EFI bootstrap under `Sources/XNV` is a retained experiment and is not the selected kernel path. Do not integrate or ship that EFI image.

Existing native programs under `Sources/` are backend prototypes. They need explicit ownership and service contracts under Darkin. XCM and Choca remain separate top-level components.

## Foundation implementation languages

The OS foundation uses C, C++ and architecture-specific assembly only. Build configuration and CI files are tooling, not OS implementation languages.

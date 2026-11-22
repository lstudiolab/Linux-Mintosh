# Mintosh native source tree

Mintosh's native foundation is implemented in C, C++ and architecture-specific assembly. **Darkin** owns the backend foundation, including the Linux-based XNV kernel integration and non-graphical system services. **XCM** and **Choca** are separate top-level components.

## Ownership

- `kernel/linux/`: upstream Linux kernel source used as the primary kernel for XNV.
- `kernel/xnv/`: XNV Linux integration policy, configuration and architecture documentation.
- `Sources/Darkin/`: Darkin ownership map, backend contracts and the XNV Linux-backed task/port/memory/event API (`Sources/Darkin/XNV/`).
- `Sources/XCM/`: external request broker, message validation and security policy.
- `Sources/Choca/`: graphical platform ownership and UI-token contract.
- `Sources/XNV/`: retained standalone EFI bootstrap experiment; not the selected XNV kernel. The actual XNV kernel is upstream Linux plus the integration configuration under `kernel/xnv/`.
- Existing backend prototypes such as Launch7762, Launch511, LibSys, Foundation, GVD, IOPS, Security and SystemLogging are Darkin-owned candidates and require migration to stable service contracts.

Choca owns USI, the window server, graphical frameworks and app manager. None of these belong inside Darkin.

## Build and test

    make verify
    make kernel-fetch
    make kernel-config
    make kernel

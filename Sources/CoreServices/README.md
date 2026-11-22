# Core Services

Core Services is Mintosh's stable shared user-space API and service layer. It contains reusable native system libraries and service contracts used by platform components and applications. It is part of the Tarts backend foundation, but remains a distinct API boundary from Tarts service supervision and XCU kernel integration.

## First implemented public API

The first Core Services compatibility API is under `include/coreservices/coreservices.h`. It exposes stable `core_*` names for the existing system status, version, service discovery and service-control API. Its initial implementation is backed by the existing CSF library and is ABI-compatible with that implementation.

The Core Services build produces and installs:
- `libcoreservices.a`
- `/usr/include/coreservices/coreservices.h`
- The compatibility declarations required by that header.

The implementation remains in `Sources/CSF/`, `Sources/LibSys/` and the aggregate native build while the API is split into independent service libraries.

## Build and test

    make -C Sources/CoreServices all
    make -C Sources/CoreServices test
    make -C Sources/CoreServices install

The test target runs the existing native Foundation/system API tests and compiles a consumer against the canonical Core Services header. This is an initial stable API surface, not the complete planned service catalogue.

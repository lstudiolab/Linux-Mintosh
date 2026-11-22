# Core Services

Core Services is Mintosh's stable shared user-space API and service layer. It contains reusable native system libraries and service contracts used by platform components and applications. It is part of the Tarts backend foundation, but remains a distinct API boundary from Tarts service supervision and kernel integration.

The initial implementation is distributed across the existing `Sources/CSF/`, `Sources/Foundation/`, `Sources/LibSys/` and selected service modules. The aggregate native build and test suite remains under `Sources/MintoshCore/` until those libraries have independent build targets and stable public headers.

Run `make -C Sources/CoreServices test` to run the existing native core test suite. This currently includes more than Core Services alone because the legacy build has not yet been split. The directory is the canonical entry point, not a claim that every planned API is complete.

# XPU

XPU is Mintosh's authenticated inter-process communication and security broker. It provides typed, versioned messages, peer authentication, capability policy, request routing and auditing between Cacao and Tarts-owned services.

The initial working broker is retained in `../XCM/` pending migration. It uses Linux Unix sequenced-packet sockets, kernel peer credentials and deny-by-default policy. The current implementation exposes read-only system information and validates presentation token streams; the renderer endpoint and broader service dispatch are not yet implemented.

Run `make -C Sources/XPU test` to build and execute the current broker tests. This is an initial broker milestone, not the complete production message bus.

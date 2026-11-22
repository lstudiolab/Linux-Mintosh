# XUM — system messaging and IPC

XUM is Lenix's system-wide messaging layer. It connects Choca, Window Server, Core Services and Sean through authenticated, typed local IPC. It is a user-space communication layer, not a second kernel.

The current broker implementation is retained under `Sources/XCM/` while its source files, public headers and protocol symbols migrate to XUM. The installed executable is still `xpu`, the compatibility client library is `libxpu.a`, and the current endpoint remains `/run/mintosh/xpu.sock`.

The broker uses Linux Unix `SOCK_SEQPACKET` sockets, kernel peer credentials and deny-by-default policy. It currently exposes read-only system information through Sean and validates USI presentation token streams. The renderer endpoint and broader service dispatch are not yet implemented.

Run `make -C Sources/XPU test` to build and execute the current broker tests. This is an initial broker milestone, not the complete production message bus.

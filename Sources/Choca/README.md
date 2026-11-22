# Choca — graphical platform

Choca is separate from Darkin. It contains all graphical and application-facing components:

- USI graphical interface and language/runtime.
- Window server and compositor.
- Graphical frameworks and controls.
- Desktop shell, Finder-like file UI, dock and launch UI.
- Application manager, package/install UI and application lifecycle client.

Choca produces a versioned USI token stream for presentation. The public token contract is `<xcm/usi_tokens.h>`: bounded text, row/column containers, spacers, buttons with numeric action IDs and images with numeric resource IDs. The stream contains no executable code, shell commands, arbitrary filesystem paths or raw system operations.

## XCM integration

Choca communicates with Darkin only through XCM. Native Choca components link the unprivileged `libxcm.a` client and include `<xcm/client.h>`. They send versioned request envelopes to `/run/mintosh/xcm.sock`; they never open Darkin's privileged service sockets directly.

XCM validates peer identity, request bounds, token structure, UTF-8, token nesting, target and granted capability before routing. The first broker milestone supports read-only system information, forwarded to Darkin over a root-only backend socket. Presentation streams are validated but return `UNAVAILABLE` until the Choca renderer registers its endpoint. No app receives permission to mutate Darkin's kernel, boot files, security policy or privileged services.

The USI language, window server, compositor and graphical implementation remain separate implementation milestones. Their absence is not represented as a completed desktop.

# Choca — graphical platform

Choca is separate from Darkin. It contains all graphical and application-facing components:

- USI graphical interface and language/runtime.
- Window server and compositor.
- Graphical frameworks and controls.
- Desktop shell, Finder-like file UI, dock and launch UI.
- Application manager, package/install UI and application lifecycle client.

Choca produces typed UI-token streams for rendering. Tokens describe presentation (text, layout, controls and visual state); they are data, not executable code or system authority.

## XCM integration

Choca communicates with Darkin only through XCM. Native Choca components link the unprivileged `libxcm.a` client and include `<xcm/client.h>`. They send versioned request envelopes to `/run/mintosh/xcm.sock`; they never open Darkin's privileged service sockets directly.

XCM validates peer identity, request bounds, token payload type, target and granted capability before routing. The first broker milestone supports read-only system information. UI-token requests are checked but return `UNAVAILABLE` until the Choca renderer registers its endpoint. No app receives permission to mutate Darkin's kernel, boot files, security policy or privileged services.

The USI language, window server, compositor and graphical implementation remain separate implementation milestones. Their absence is not represented as a completed desktop.

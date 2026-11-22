# Choca — graphical platform

Choca is separate from Darkin. It contains all graphical and application-facing components:

- USI graphical interface and language/runtime.
- Window server and compositor.
- Graphical frameworks and controls.
- Desktop shell, Finder-like file UI, dock and launch UI.
- Application manager, package/install UI and application lifecycle client.

Choca produces typed UI-token streams for rendering. Tokens describe presentation (for example text, layout, controls and state) and do not carry kernel or system privileges. XCM validates the token envelope and routes it only to the rendering endpoint.

Choca communicates with Darkin only through XCM. It cannot access privileged Darkin APIs directly. The USI language and graphical implementation remain deferred until the language/toolchain and XCM protocol are stable.

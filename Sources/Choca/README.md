# Cacao — graphical platform (legacy source location)

The platform's final name is **Cacao**. This directory remains named `Sources/Choca/` temporarily so existing references and build integration are not broken during migration.

Cacao owns the graphical platform and native application framework:
- Desktop-facing graphical experience and application manager.
- Native application lifecycle and event-loop APIs.
- Standard controls, views, layout, drawing and accessibility.
- Client APIs for the separate Window Server.
- Public application SDK and common graphical services.

Window Server remains a distinct service responsible for displays, windows, surfaces, composition, focus and input routing. Cacao uses its documented client protocol; it does not directly control privileged display devices.

Cacao communicates with backend services through the XPU interface. The current XPU prototype is still implemented under `Sources/XCM/`. USI token validation remains a presentation-data contract, not an executable or privileged command format.

The actual graphical framework, desktop, app manager and Window Server are not yet implemented. This file records ownership and migration direction; it does not claim those components are complete.
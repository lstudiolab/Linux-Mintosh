# Cacao — legacy source root

Cacao is Mintosh's native graphical and application platform. `Sources/Cacao/` is the canonical future source root; this old Choca path is migration debt, not a separate component.

Cacao owns:
- Native application lifecycle and event-loop APIs.
- ApplicationKit: windows, menus, dialogs and standard controls.
- Views, layout, drawing, text, animation and accessibility.
- Localization, preferences, document and resource APIs.
- The desktop shell and application manager.
- The public native application SDK and application bundle contract.

The **Window Server is separate**. It owns displays, windows, surfaces, composition, focus and input routing. Cacao uses its client protocol and must not access privileged display devices directly.

Cacao communicates with backend services through Core Services and XPU. XPU is the authenticated cross-process capability boundary, not a replacement for service APIs.

The framework, Window Server, desktop, app manager and SDK are not yet implemented. New Cacao code belongs under `Sources/Cacao/`.

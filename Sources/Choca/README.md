# Choca

Choca is Lenix's native graphical platform and application framework. Its canonical source root is `Sources/Choca/`.

Choca owns:
- Native application lifecycle and event loops.
- ApplicationKit: windows, menus, dialogs and standard controls.
- Views, layout, drawing, text, animation and accessibility.
- Localization, preferences, document and resource APIs.
- The desktop shell, application manager and public application SDK.

The **Window Server is a separate system service**. It owns displays, windows, surfaces, composition, focus and input routing. Choca uses its client protocol and must not access privileged display devices directly.

Choca communicates with backend services through Core Services and XUM. XUM is the authenticated cross-process messaging boundary; Core Services provides stable service APIs.

The framework, Window Server, desktop, application manager and SDK are not yet implemented. New graphical-platform code belongs under `Sources/Choca/`.

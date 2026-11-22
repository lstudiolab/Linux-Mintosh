# Cacao

Cacao is the native application and graphical framework for Mintosh. Its canonical ownership is defined in [the platform architecture](../../docs/platform-architecture.md).

Cacao will provide ApplicationKit, application lifecycle, event loops, standard controls, views, layout, drawing, text, accessibility, localization, resource/document APIs, the desktop shell and application manager.

Cacao is a client of the separate Window Server. It uses Core Services for stable system APIs and XPU for authenticated cross-process capabilities. It must not access privileged display devices or private Tarts sockets directly.

The framework is not yet implemented. This directory is the canonical source root for new Cacao implementation.

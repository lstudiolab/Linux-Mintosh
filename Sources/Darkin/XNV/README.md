# Deprecated XNV source copy

XNV has been rebranded and redesigned as **XCU**. This directory is a frozen migration reference and is not the canonical implementation or build path.

The canonical source, public API and tests are now under `Sources/XCU/`. New code must use `<xcu/xcu.h>`, `xcu_*` symbols and `libxcu.a`. Do not add features here or link `libxnv.a`.

XCU uses Linux as its actual kernel. BSD-derived interfaces and Mach-inspired abstractions are implemented using Linux facilities; there is no independent XNV/Mach/BSD kernel.

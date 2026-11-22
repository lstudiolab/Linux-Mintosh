# Retired standalone EFI kernel experiment

This directory contains an early freestanding EFI/kernel experiment from before Mintosh selected Linux as its actual kernel. It is **not XCU**, is not part of the selected architecture and must not be used as a second kernel or boot path.

The current Mintosh architecture boots the Linux kernel through mBoot. XCU is the renamed and redesigned continuation of the Linux-backed XNV integration layer; its canonical implementation is under `Sources/XCU/`.

The old experiment is retained temporarily for historical reference only. Do not add features, include its headers, or connect it to the current build. Its GNU-EFI dependency and custom GDT/IDT, paging and memory bootstrap are not requirements of the Linux-based Mintosh foundation.

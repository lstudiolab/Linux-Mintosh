#!/usr/bin/env python3
"""Validate the Mintosh release manifest without downloading build inputs."""
import json
import re
import sys
from pathlib import Path
from urllib.parse import urlparse

ROOT = Path(__file__).resolve().parents[1]
manifest_path = ROOT / "configs" / "release.json"


def fail(message: str) -> None:
    print(f"release manifest: {message}", file=sys.stderr)
    raise SystemExit(1)


try:
    data = json.loads(manifest_path.read_text(encoding="utf-8"))
except (OSError, json.JSONDecodeError) as exc:
    fail(str(exc))

if data.get("distribution") != "Mintosh":
    fail("distribution must be Mintosh")

version = data.get("release", "")
semver = r"(?:0|[1-9][0-9]*)\.(?:0|[1-9][0-9]*)\.(?:0|[1-9][0-9]*)"
if not isinstance(version, str) or not re.fullmatch(semver, version):
    fail("release must use semantic versioning (MAJOR.MINOR.PATCH)")

suite = data.get("debian_suite")
if not isinstance(suite, str) or not suite:
    fail("debian_suite must be a non-empty string")

architectures = data.get("architectures")
if not isinstance(architectures, list) or not architectures:
    fail("architectures must list at least one bootable installer target")
if len(set(architectures)) != len(architectures):
    fail("architectures contains duplicates")
if not set(architectures).issubset({"x86_64", "arm64"}):
    fail("unknown bootable architecture")

kernel_build_architectures = data.get("kernel_build_architectures")
if not isinstance(kernel_build_architectures, list) or not set(architectures).issubset(set(kernel_build_architectures)):
    fail("kernel_build_architectures must include every bootable architecture")
if len(set(kernel_build_architectures)) != len(kernel_build_architectures):
    fail("kernel_build_architectures contains duplicates")
if not set(kernel_build_architectures).issubset({"x86_64", "arm64"}):
    fail("unknown kernel build architecture")

planned_architectures = data.get("planned_architectures", [])
if not isinstance(planned_architectures, list) or set(planned_architectures) & set(kernel_build_architectures):
    fail("planned_architectures must be separate from current kernel build targets")

kernel = data.get("kernel", {})
kernel_version = kernel.get("version", "")
if not isinstance(kernel_version, str) or not re.fullmatch(r"[0-9]+\.[0-9]+\.[0-9]+", kernel_version):
    fail("kernel.version must be an exact upstream version")
if kernel.get("archive") != f"linux-{kernel_version}.tar.xz":
    fail("kernel.archive must match kernel.version")
url = kernel.get("source", "")
parsed = urlparse(url)
if parsed.scheme != "https" or parsed.hostname != "cdn.kernel.org":
    fail("kernel.source must use the official HTTPS kernel.org CDN")
if not parsed.path.endswith(f"/{kernel['archive']}"):
    fail("kernel.source filename must match kernel.archive")
checksum = kernel.get("sha256", "")
if not isinstance(checksum, str) or not re.fullmatch(r"[0-9a-f]{64}", checksum):
    fail("kernel.sha256 must contain exactly 64 lowercase hexadecimal characters")
if kernel.get("license") != "GPL-2.0-only":
    fail("kernel.license must identify the upstream kernel license")

image = data.get("image", {})
bootable_architectures = image.get("bootable_architectures")
if bootable_architectures != architectures:
    fail("image.bootable_architectures must exactly match verified installer architectures")
if image.get("primary_architecture") not in bootable_architectures:
    fail("image.primary_architecture must be a verified bootable architecture")
if image.get("bootloader") != "GRUB":
    fail("the initial bootloader must be GRUB")
if not {"BIOS", "UEFI"}.issubset(set(image.get("firmware", []))):
    fail("the x86_64 image must declare BIOS and UEFI support")

print(f"Mintosh {version}: manifest valid")
print(f"Linux {kernel_version}: SHA-256 pin present")
print(f"Bootable architectures: {', '.join(architectures)}")
print(f"Kernel build architectures: {', '.join(kernel_build_architectures)}")
print(f"Planned architectures: {', '.join(planned_architectures) or 'none'}")

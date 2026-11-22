#!/usr/bin/env python3
import pathlib
import struct
import sys

if len(sys.argv) < 2:
    raise SystemExit("usage: test-mboot.py EFI_IMAGE [EFI_IMAGE ...]")

for filename in sys.argv[1:]:
    path = pathlib.Path(filename)
    data = path.read_bytes()
    if len(data) < 256 or data[:2] != b"MZ":
        raise SystemExit(f"{path}: not a PE/COFF EFI image")
    pe_offset = struct.unpack_from("<I", data, 0x3C)[0]
    if pe_offset + 24 > len(data) or data[pe_offset:pe_offset + 4] != b"PE\0\0":
        raise SystemExit(f"{path}: invalid PE signature")
    machine, _, _, _, _, optional_size, _ = struct.unpack_from(
        "<HHIIIHH", data, pe_offset + 4
    )
    if machine != 0x8664:
        raise SystemExit(f"{path}: machine type is not x86-64")
    optional = pe_offset + 24
    if optional + optional_size > len(data) or struct.unpack_from("<H", data, optional)[0] != 0x20B:
        raise SystemExit(f"{path}: missing PE32+ optional header")
    subsystem = struct.unpack_from("<H", data, optional + 68)[0]
    if subsystem != 10:
        raise SystemExit(f"{path}: subsystem is not an EFI application")
    print(f"{path.name}: mBoot x86-64 EFI application verified ({len(data)} bytes)")

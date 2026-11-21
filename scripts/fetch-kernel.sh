#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
ARCH=${ARCH:-x86_64}
DEST="$ROOT/kernel/linux"
WORK="$ROOT/out/downloads"
MANIFEST="$ROOT/configs/release.json"
case "$ARCH" in x86_64|arm64) ;; *) echo "Unsupported architecture: $ARCH" >&2; exit 1 ;; esac
[ -f "$MANIFEST" ] || { echo "Missing release manifest: $MANIFEST" >&2; exit 1; }
[ ! -e "$DEST/Makefile" ] || { echo "Kernel source already exists at $DEST"; exit 0; }
VERSION=$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1]))["kernel"]["version"])' "$MANIFEST")
URL=$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1]))["kernel"]["source"])' "$MANIFEST")
ARCHIVE="$WORK/linux-$VERSION.tar.xz"
TMP="$ROOT/kernel/.linux-$VERSION.tmp"
mkdir -p "$WORK" "$ROOT/kernel"
echo "Fetching Mintosh-pinned Linux $VERSION for $ARCH..."
curl --fail --location --show-error "$URL" -o "$ARCHIVE"
rm -rf "$TMP"
mkdir -p "$TMP"
tar -xJf "$ARCHIVE" --strip-components=1 -C "$TMP"
mv "$TMP" "$DEST"
printf '%s\n' "$VERSION" > "$ROOT/kernel/LINUX_VERSION"
rm -f "$ARCHIVE"
echo "Linux $VERSION source is ready at kernel/linux."

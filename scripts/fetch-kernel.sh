#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
ARCH=${ARCH:-x86_64}
DEST="$ROOT/kernel/linux"
WORK="$ROOT/out/downloads"
RELEASES="$WORK/releases.json"

case "$ARCH" in
  x86_64|arm64) ;;
  *) echo "Unsupported architecture: $ARCH" >&2; exit 1 ;;
esac

if [ -d "$DEST/.git" ] || [ -f "$DEST/Makefile" ]; then
  echo "Kernel source already exists at $DEST"
  echo "Remove that directory if you want to fetch a fresh source tree."
  exit 0
fi

mkdir -p "$WORK" "$ROOT/kernel"
echo "Checking kernel.org for the latest stable Linux release..."
curl --fail --location --silent --show-error \
  https://www.kernel.org/releases.json -o "$RELEASES"

VERSION=$(python3 - "$RELEASES" <<'PY'
import json, sys
with open(sys.argv[1], encoding="utf-8") as f:
    data = json.load(f)
releases = data.get("releases", [])
stable = next((r for r in releases if r.get("moniker") == "stable"), None)
if not stable or not stable.get("version"):
    raise SystemExit("Could not identify the current stable kernel release.")
print(stable["version"])
PY
)

case "$VERSION" in
  *[!0-9.]*|'') echo "Invalid kernel version from kernel.org: $VERSION" >&2; exit 1 ;;
esac

MAJOR=${VERSION%%.*}
case "$MAJOR" in
  ''|*[!0-9]*) echo "Invalid kernel major version: $MAJOR" >&2; exit 1 ;;
esac

URL="https://cdn.kernel.org/pub/linux/kernel/v${MAJOR}.x/linux-${VERSION}.tar.xz"
ARCHIVE="$WORK/linux-$VERSION.tar.xz"
TMP="$ROOT/kernel/.linux-$VERSION.tmp"

echo "Downloading Linux $VERSION..."
curl --fail --location --show-error "$URL" -o "$ARCHIVE"

echo "Extracting Linux $VERSION..."
rm -rf "$TMP"
mkdir -p "$TMP"
tar -xJf "$ARCHIVE" --strip-components=1 -C "$TMP"
mv "$TMP" "$DEST"
rm -f "$ARCHIVE"

printf '%s\n' "$VERSION" > "$ROOT/kernel/LINUX_VERSION"
echo "Linux $VERSION source is ready at kernel/linux."

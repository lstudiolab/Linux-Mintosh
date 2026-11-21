#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)
make -C "$ROOT/Sources/MintoshCore" clean all
B="$ROOT/Sources/MintoshCore/build"
"$B/mosf" --version | grep -q '0.1.0'
"$B/launch7762" --version | grep -q '0.1.0'
"$B/mintoshctl" --version | grep -q '0.1.0'
"$B/ioplus" --version | grep -q '0.1.0'
"$B/kextctl" --version | grep -q '0.1.0'
"$B/mosflog" --version | grep -q '0.1.0'
"$B/security-audit" --version | grep -q '0.1.0'
echo "Mintosh native core build and CLI smoke tests passed."

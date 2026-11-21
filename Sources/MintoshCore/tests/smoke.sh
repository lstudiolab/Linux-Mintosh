#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)
make -C "$ROOT/Sources/MintoshCore" clean all
B="$ROOT/Sources/MintoshCore/build"

"$B/mosf" --version | grep -q '0.1.0'
"$B/launch7762" --version | grep -q '0.1.0'
set +e
"$B/launch511" >/dev/null 2>&1
LAUNCH511_STATUS=$?
set -e
[ "$LAUNCH511_STATUS" -eq 64 ]
"$B/ioplus" --version | grep -q '0.1.0'
"$B/kextctl" --version | grep -q '0.1.0'
"$B/mosflog" --version | grep -q '0.1.0'
"$B/security-audit" --version | grep -q '0.1.0'
"$B/mintoshctl" --version | grep -q '0.1.0'

# Exercise the real MOSF Unix-socket protocol, not just its version flag.
"$B/mosf" &
MOSF_PID=$!
cleanup() {
    kill "$MOSF_PID" 2>/dev/null || true
    wait "$MOSF_PID" 2>/dev/null || true
}
trap cleanup EXIT INT TERM
attempt=0
while [ ! -S /run/mintosh/mosf.sock ]; do
    attempt=$((attempt + 1))
    [ "$attempt" -lt 50 ] || { echo "MOSF socket did not appear" >&2; exit 1; }
    sleep 0.1
done
"$B/mintoshctl" status | grep -q 'MOSF=running'
"$B/mintoshctl" version | grep -q 'Mintosh 0.1.0'
cleanup
trap - EXIT INT TERM
echo "Mintosh native source build and runtime integration tests passed."

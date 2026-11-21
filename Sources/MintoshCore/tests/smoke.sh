#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)
make -C "$ROOT/Sources/MintoshCore" clean all
B="$ROOT/Sources/MintoshCore/build"
RUNTIME=$(mktemp -d)
export MINTOSH_RUN_DIR="$RUNTIME"
cleanup() {
    if [ -n "${MOSF_PID:-}" ]; then
        kill "$MOSF_PID" 2>/dev/null || true
        wait "$MOSF_PID" 2>/dev/null || true
    fi
    rm -rf "$RUNTIME"
}
trap cleanup EXIT INT TERM

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

# Exercise the real MOSF Unix-socket protocol in an unprivileged temp directory.
"$B/mosf" &
MOSF_PID=$!
attempt=0
while [ ! -S "$RUNTIME/mosf.sock" ]; do
    attempt=$((attempt + 1))
    [ "$attempt" -lt 50 ] || { echo "MOSF socket did not appear" >&2; exit 1; }
    sleep 0.1
done
"$B/mintoshctl" status | grep -q 'MOSF=running'
"$B/mintoshctl" version | grep -q 'Mintosh 0.1.0'
cleanup
unset MOSF_PID
trap - EXIT INT TERM
echo "Mintosh native source build and runtime integration tests passed."

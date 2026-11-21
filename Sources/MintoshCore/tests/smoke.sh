#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)
make -C "$ROOT/Sources/MintoshCore" clean all
B="$ROOT/Sources/MintoshCore/build"
RUNTIME=$(mktemp -d)
export MINTOSH_RUN_DIR="$RUNTIME"
export MINTOSH_SERVICE_DIR="$RUNTIME/services"
mkdir -p "$MINTOSH_SERVICE_DIR"
cleanup() {
    if [ -n "${LAUNCH_PID:-}" ]; then
        kill "$LAUNCH_PID" 2>/dev/null || true
        wait "$LAUNCH_PID" 2>/dev/null || true
    fi
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

# Exercise the real MOSF Unix-socket protocol as an unprivileged user.
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

# Exercise service parsing, process lifecycle, restart policy and socket control.
cat > "$MINTOSH_SERVICE_DIR/test-service.service" <<EOF
ExecStart=/bin/sleep 30
Enabled=yes
Restart=no
EOF
cat > "$MINTOSH_SERVICE_DIR/retry-service.service" <<EOF
ExecStart=printf x >> $RUNTIME/restarts; exit 1
Enabled=yes
Restart=on-failure
EOF
cat > "$MINTOSH_SERVICE_DIR/unsafe.service" <<EOF
ExecStart=/bin/sleep 30
Enabled=yes
Restart=always
EOF
chmod 0666 "$MINTOSH_SERVICE_DIR/unsafe.service"

"$B/launch7762" &
LAUNCH_PID=$!
attempt=0
while [ ! -S "$RUNTIME/launch.sock" ]; do
    attempt=$((attempt + 1))
    [ "$attempt" -lt 50 ] || { echo "launch7762 socket did not appear" >&2; exit 1; }
    sleep 0.1
done
sleep 0.3
"$B/mintoshctl" services | grep -q 'test-service running'
if "$B/mintoshctl" services | grep -q '^unsafe '; then
    echo "launch7762 loaded an unsafe service definition" >&2
    exit 1
fi

# A failed service with on-failure policy must be restarted.
attempt=0
while [ ! -f "$RUNTIME/restarts" ] || [ "$(wc -c < "$RUNTIME/restarts")" -lt 2 ]; do
    attempt=$((attempt + 1))
    [ "$attempt" -lt 60 ] || { echo "on-failure service was not restarted" >&2; exit 1; }
    sleep 0.1
done
"$B/mintoshctl" stop retry-service | grep -q 'OK'
"$B/mintoshctl" stop test-service | grep -q 'OK'
cleanup
unset MOSF_PID LAUNCH_PID
trap - EXIT INT TERM
echo "Mintosh native build, MOSF protocol, service security and supervision tests passed."

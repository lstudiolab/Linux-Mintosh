#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../../.." && pwd)
make -C "$ROOT/Sources/MintoshCore" clean all test-foundation
B="$ROOT/Sources/MintoshCore/build"
RUNTIME=$(mktemp -d)
export MINTOSH_RUN_DIR="$RUNTIME"
export MINTOSH_SERVICE_DIR="$RUNTIME/services"
mkdir -m 0755 "$MINTOSH_SERVICE_DIR"
cleanup() {
    if [ -n "${LAUNCH_PID:-}" ]; then
        kill "$LAUNCH_PID" 2>/dev/null || true
        wait "$LAUNCH_PID" 2>/dev/null || true
    fi
    if [ -n "${Darl_PID:-}" ]; then
        kill "$Darl_PID" 2>/dev/null || true
        wait "$Darl_PID" 2>/dev/null || true
    fi
    rm -rf "$RUNTIME"
}
trap cleanup EXIT INT TERM

"$B/darl" --version | grep -q 'Darl 0.2.0'
"$B/launch7762" --version | grep -q '0.2.0'
set +e
"$B/launch511" >/dev/null 2>&1
LAUNCH511_STATUS=$?
"$B/kextctl" load --help >/dev/null 2>&1
KEXT_INVALID_STATUS=$?
set -e
[ "$LAUNCH511_STATUS" -eq 64 ]
[ "$KEXT_INVALID_STATUS" -eq 64 ]
"$B/ioplus" --version | grep -q '0.2.0'
"$B/ioplus" --json | python3 -c 'import json,sys; value=json.load(sys.stdin); assert {"kernel","memory_kib","devices"} <= value.keys()'
"$B/kextctl" --version | grep -q '0.1.0'
"$B/mintosh-log" --version | grep -q '0.1.0'
"$B/security-audit" --version | grep -q '0.3.0'
"$B/mintoshctl" --version | grep -q '0.1.0'
"$B/mstore" --version | grep -q '0.1.0'
set +e
"$B/mstore" install --invalid >/dev/null 2>&1
MSTORE_INVALID_STATUS=$?
set -e
[ "$MSTORE_INVALID_STATUS" -eq 64 ]

"$B/darl" &
Darl_PID=$!
attempt=0
while [ ! -S "$RUNTIME/darl.sock" ]; do
    attempt=$((attempt + 1))
    [ "$attempt" -lt 50 ] || { echo "Darl socket did not appear" >&2; exit 1; }
    sleep 0.1
done
"$B/mintoshctl" status | grep -q 'Darl=running'
"$B/mintoshctl" version | grep -q 'Mintosh 0.1.0'
[ "$(stat -c '%a' "$RUNTIME/darl.sock")" = "660" ]

cat > "$MINTOSH_SERVICE_DIR/test-service.service" <<EOF
ExecStart=/bin/sleep 30
Enabled=yes
Restart=no
EOF
cat > "$RUNTIME/retry-service.sh" <<EOF
#!/bin/sh
echo x >> "$RUNTIME/restarts"
exit 1
EOF
chmod 0755 "$RUNTIME/retry-service.sh"
cat > "$MINTOSH_SERVICE_DIR/retry-service.service" <<EOF
ExecStart=/bin/sh $RUNTIME/retry-service.sh
Enabled=yes
Restart=on-failure
EOF
cat > "$MINTOSH_SERVICE_DIR/unsafe.service" <<EOF
ExecStart=/bin/sleep 30
Enabled=yes
Restart=always
EOF
chmod 0666 "$MINTOSH_SERVICE_DIR/unsafe.service"
cat > "$MINTOSH_SERVICE_DIR/shell-string.service" <<EOF
ExecStart=echo x; /bin/sh -c id
Enabled=yes
Restart=no
EOF

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
if "$B/mintoshctl" services | grep -Eq '^(unsafe|shell-string) '; then
    echo "launch7762 loaded an unsafe service definition" >&2
    exit 1
fi
[ "$(stat -c '%a' "$RUNTIME/launch.sock")" = "660" ]

attempt=0
while [ ! -f "$RUNTIME/restarts" ] || [ "$(wc -c < "$RUNTIME/restarts")" -lt 2 ]; do
    attempt=$((attempt + 1))
    [ "$attempt" -lt 100 ] || { echo "on-failure service was not restarted" >&2; exit 1; }
    sleep 0.1
done
attempt=0
while ! "$B/mintoshctl" services | grep -Eq 'retry-service (running|restarting|stopped).*restarts=2'; do
    attempt=$((attempt + 1))
    [ "$attempt" -lt 30 ] || { echo "restart backoff state was not recorded" >&2; exit 1; }
    sleep 0.1
done
"$B/mintoshctl" stop retry-service | grep -q 'OK'
"$B/mintoshctl" stop test-service | grep -q 'OK'
cleanup
unset Darl_PID LAUNCH_PID
trap - EXIT INT TERM
echo "Mintosh native build, Foundation, Darl protocol, service security and supervision tests passed."

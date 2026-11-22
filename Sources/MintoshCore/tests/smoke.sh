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
"$B/launch-client" --version | grep -q '0.1.0'
set +e
env -u XDG_RUNTIME_DIR HOME=/tmp "$B/launch-client" >/dev/null 2>&1
LAUNCH_CLIENT_ENV_STATUS=$?
env -u XDG_RUNTIME_DIR "$B/mintoshctl" --user services >/dev/null 2>&1
MINTOSHCTL_USER_ENV_STATUS=$?
set -e
[ "$LAUNCH_CLIENT_ENV_STATUS" -eq 2 ]
[ "$MINTOSHCTL_USER_ENV_STATUS" -eq 2 ]
set +e
"$B/launch511" >/dev/null 2>&1
LAUNCH511_STATUS=$?
"$B/kextctl" load --help >/dev/null 2>&1
KEXT_INVALID_STATUS=$?
set -e
[ "$LAUNCH511_STATUS" -eq 64 ]
[ "$KEXT_INVALID_STATUS" -eq 64 ]
"$B/hp-compat" --version | grep -q '0.1.0'
"$B/hp-compat" --json | python3 -c 'import json,sys; value=json.load(sys.stdin); assert {"vendor","product","board","bios_version","cpu_vendor","is_hp","is_probook"} <= value.keys() and value["cpu_vendor"]'
"$B/ioplus" --version | grep -q '0.2.0'
"$B/gvdctl" --version | grep -q '0.1.0'
"$B/gvdctl" list --json | python3 -c 'import json,sys; value=json.load(sys.stdin); assert isinstance(value,list) and value'
"$B/iopsctl" --version | grep -q '0.1.0'
"$B/iopsctl" status --json | python3 -c 'import json,sys; value=json.load(sys.stdin); assert {"power_supplies","thermal_zones","cpu_governors","platform_profile"} <= value.keys()'
"$B/fmsctl" --version | grep -q '0.1.0'
"$B/dmctl" --version | grep -q '0.1.0'
"$B/dmctl" list --json | python3 -c 'import json,sys; value=json.load(sys.stdin); assert isinstance(value,list)'
mkdir -p "$RUNTIME/fms-tree/subdir"
printf 'Mintosh metadata search fixture\\n' > "$RUNTIME/fms-tree/subdir/mintosh-search-fixture.txt"
MINTOSH_FMS_DB="$RUNTIME/fms.db" "$B/fmsctl" index "$RUNTIME/fms-tree" | grep -q 'Indexed 1 files'
MINTOSH_FMS_DB="$RUNTIME/fms.db" "$B/fmsctl" search mintosh --json | python3 -c 'import json,sys; value=json.load(sys.stdin); assert len(value)==1 and value[0]["path"].endswith("mintosh-search-fixture.txt")'
"$B/ioplus" --json | python3 -c 'import json,sys; value=json.load(sys.stdin); assert {"kernel","cpu_model","memory_kib","firmware","devices"} <= value.keys(); assert {"block","network","usb","graphics","power"} <= value["devices"].keys()' 
set +e
"$B/ioplus" --json unexpected >/dev/null 2>&1
IOPLUS_INVALID_STATUS=$?
set -e
[ "$IOPLUS_INVALID_STATUS" -eq 64 ]
set +e
"$B/dmctl" mount /dev/not-a-block /mnt >/dev/null 2>&1
DM_INVALID_MOUNT_STATUS=$?
"$B/dmctl" unmount /etc >/dev/null 2>&1
DM_UNSAFE_UNMOUNT_STATUS=$?
set -e
[ "$DM_INVALID_MOUNT_STATUS" -eq 64 ]
[ "$DM_UNSAFE_UNMOUNT_STATUS" -eq 64 ]
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
cat > "$RUNTIME/environment-service.sh" <<EOF
#!/bin/sh
printf '%s' "\${MINTOSH_TEST_ENV:-missing}" > "$RUNTIME/inherited-environment"
EOF
chmod 0755 "$RUNTIME/environment-service.sh"
export MINTOSH_TEST_ENV="mintosh-user-environment"
cat > "$RUNTIME/retry-service.sh" <<EOF
#!/bin/sh
echo x >> "$RUNTIME/restarts"
exit 1
EOF
chmod 0755 "$RUNTIME/retry-service.sh"
cat > "$MINTOSH_SERVICE_DIR/environment-service.service" <<EOF
ExecStart=/bin/sh $RUNTIME/environment-service.sh
Enabled=yes
Restart=no
EOF
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
attempt=0
while [ ! -f "$RUNTIME/inherited-environment" ]; do
    attempt=$((attempt + 1))
    [ "$attempt" -lt 50 ] || { echo "user service environment was not inherited" >&2; exit 1; }
    sleep 0.1
done
[ "$(cat "$RUNTIME/inherited-environment")" = "$MINTOSH_TEST_ENV" ]
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

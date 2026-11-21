# Launch Client

Launch Client is the per-user background service manager entry point. It starts the shared launch7762 supervisor with user-owned service definitions under `$XDG_CONFIG_HOME/mintosh/services` (or `$HOME/.config/mintosh/services`) and a private control socket under `$XDG_RUNTIME_DIR/mintosh`.

The launcher requires absolute HOME and XDG_RUNTIME_DIR values, creates directories without following symlink components, and verifies their ownership before starting the supervisor. It runs without root privileges; child services inherit the user's UID and cannot control another user's supervisor.

The system-wide Launch Background supervisor remains launch7762 under systemd. User services are managed with:

    mintoshctl --user services
    mintoshctl --user start SERVICE
    mintoshctl --user stop SERVICE
    mintoshctl --user restart SERVICE

The user systemd unit is enabled by default for newly created and existing user sessions by the Mintosh base image.

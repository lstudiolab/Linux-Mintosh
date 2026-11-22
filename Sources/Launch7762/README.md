# Tarts service supervisor — launch7762 (legacy executable name)

launch7762 is Mintosh's native userspace service supervisor and the initial Tarts service-lifecycle implementation. It loads trusted service definitions, starts enabled services in deterministic order, reaps child processes, applies restart policies with capped exponential backoff, and exposes a local Unix-socket control API.

## Service definition

A service definition is a root-owned regular `.service` file in a directory that is not group- or world-writable:

```ini
ExecStart=/usr/bin/xpu
Enabled=yes
Restart=always
Group=mintosh-services
Order=20
```

- `ExecStart` is a whitespace-separated argument vector. It must begin with an absolute executable path. No shell syntax, quoting, variable expansion, pipes, redirects or command substitutions are interpreted.
- `Enabled` is `yes` or `no`.
- `Restart` is `no`, `on-failure` or `always`.
- `Group` optionally selects a primary group for the service. A root supervisor clears inherited supplementary groups and applies the configured GID before executing the service.
- `Order` optionally selects startup order from 0 through 9999. Lower values start first; ties are sorted by service name. The default is 10000.

The executable and every parent directory are validated component-by-component with `openat(O_NOFOLLOW)`. Parent directories and the final executable must be trusted and not group/world writable. Common shell and scripting interpreters are rejected; privileged services must be native executables.

Restart delays use capped exponential backoff. Repeated failures eventually disable a service rather than restarting forever.

## Control API

`mintoshctl services` lists service states. `start`, `stop` and `restart` require the local daemon owner or root; the socket's group permission grants listing access, not service-control authority.

The Tarts service manager is an early native implementation. It still needs explicit dependency graphs, readiness notifications, resource accounting, structured logs and per-service capability profiles before it can replace all system service-management responsibilities.

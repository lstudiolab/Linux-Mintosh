# launch7762

launch7762 is Mintosh's native userspace service supervisor. It loads root-owned service definitions from `/etc/mintosh/services`, starts enabled services, reaps child processes, applies restart policies with capped exponential backoff, and exposes a local Unix-socket control API.

## Service definition

A service definition is a root-owned regular `.service` file in a directory that is not group- or world-writable:

```ini
ExecStart=/usr/bin/darl
Enabled=yes
Restart=always
```

`ExecStart` is a whitespace-separated argument vector. The executable must be an absolute path. launch7762 does not interpret shell syntax, quotes, variable expansion, pipes, redirects, or command substitutions. Use a dedicated executable or interpreter explicitly when a script is required.

Supported restart policies are `no`, `on-failure`, and `always`. Repeated failures use exponential restart delays capped at 60 seconds.

## Control API

`mintoshctl services` lists service states. `start`, `stop`, and `restart` require the local daemon owner or root; the socket's group permission grants listing access, not service-control authority.

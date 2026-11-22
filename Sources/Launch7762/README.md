# launch7762

launch7762 is Mintosh's native userspace service supervisor. It loads root-owned service definitions from `/etc/mintosh/services`, starts enabled services, reaps child processes, applies restart policies with capped exponential backoff, and exposes a local Unix-socket control API.

## Service definition

A service definition is a root-owned regular `.service` file in a directory that is not group- or world-writable:

```ini
ExecStart=/usr/bin/darl
Enabled=yes
Restart=always
```

`ExecStart` is a whitespace-separated argument vector. The executable must be an absolute path. launch7762 does not interpret shell syntax, quotes, variable expansion, pipes, redirects, or command substitutions. Service definitions are restricted to native executable programs; script interpreters are rejected so user-writable script files cannot be executed with service-manager privileges.

Supported restart policies are `no`, `on-failure`, and `always`. Repeated failures use exponential restart delays capped at 60 seconds.

## Control API

`mintoshctl services` lists service states. `start`, `stop`, and `restart` require the local daemon owner or root; the socket's group permission grants listing access, not service-control authority.


## Executable trust boundary

Before forking a service, launch7762 validates the executable path component-by-component using openat with O_NOFOLLOW. Every parent directory must be a trusted, non-group/world-writable directory, and the final target must be a trusted regular executable that is not group/world-writable. This prevents service definitions from launching binaries out of user-writable paths such as /tmp. The service executable is still executed by absolute path; trusted directory ownership and write permissions are therefore part of the security boundary.

The executable policy rejects common shell and scripting interpreters. Mintosh system services are expected to be native programs; scripts are not a supported privileged service format.

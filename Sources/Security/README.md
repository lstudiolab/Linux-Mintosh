# Security

The initial Security baseline audits critical account-file permissions, Mintosh service-directory ownership and write permissions, service-definition files, and ownership/modes of installed Mintosh executables.

Service definitions must be regular root-owned files without group/world write permission and must remain within the launch7762 parser's 64 KiB limit. The audit rejects symlinks and unexpected entries. It is a configuration-integrity check, not a complete MAC policy, sandbox, capability system or vulnerability scanner.

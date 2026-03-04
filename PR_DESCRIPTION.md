# Pull Request: Add TLS support for agent-manager channel (NIS2 compliant)

Copy the content below into the PR description when opening the pull request on GitHub.

---

## Description

Adds TLS/SSL encryption to the agent-manager data channel to satisfy NIS2 (and similar) directive requirements for device-to-information-system communication. The default agent-manager channel uses AES over plain TCP; this change adds an optional TLS mode for compliance.

## Proposed Changes

### Manager
- New `<remote>` connection type `secure_tls` listening on port 1516 (configurable)
- TLS certificate, key, and CA configuration
- Backward compatible: existing `secure` (port 1514) continues to work

### Agent
- New `<manager>` options: `use_tls`, `tls_port`, `tls_certificate_path`, `tls_key_path`, `tls_ca_path`
- Native TLS client via `OS_ConnectTLS` in os_net
- TLS-aware `OS_SendSecureTCP` / `OS_RecvSecureTCP` when socket is TLS-wrapped
- Falls back to enrollment cert paths when TLS paths not explicitly set

### Shared
- `os_net`: TLS socket map, `OS_ConnectTLS`, SSL read/write in secure TCP helpers
- `defs.h`: `DEFAULT_SECURE_TLS` (1516)

### Results and Evidence

- Compilation verified on Linux (or document your platform)
- Manual test: Manager with `secure_tls` on 1516, agent with `use_tls` connects successfully
- Existing AES (port 1514) connections unaffected

### Manual tests with their corresponding evidence

- [ ] Compilation without warnings on every supported platform
  - [ ] Linux
  - [ ] Windows
  - [ ] MAC OS X
- [ ] Log syntax and correct language review

### Artifacts Affected

- `src/config/` – remote-config, client-config
- `src/remoted/` – remoted.c, secure.c, netbuffer.c, config.c
- `src/shared/os_net/` – os_net.c, os_net.h
- `src/client-agent/` – start_agent.c
- `src/shared/include/defs.h`

### Configuration Changes

**Manager (ossec.conf):**
```xml
<remote>
  <connection>secure_tls</connection>
  <port>1516</port>
  <tls_certificate>/var/ossec/etc/sslmanager.cert</tls_certificate>
  <tls_key>/var/ossec/etc/sslmanager.key</tls_key>
  <tls_ca>/var/ossec/etc/rootca.cert</tls_ca>
</remote>
```

**Agent (ossec.conf):**
```xml
<manager>
  <address>manager-ip</address>
  <port>1514</port>
  <use_tls>yes</use_tls>
  <tls_port>1516</tls_port>
  <tls_certificate_path>/var/ossec/etc/agent.cert</tls_certificate_path>
  <tls_key_path>/var/ossec/etc/agent.key</tls_key_path>
  <tls_ca_path>/var/ossec/etc/rootca.cert</tls_ca_path>
</manager>
```

### Tests Introduced

- No new automated tests in this PR; existing remoted integration tests remain valid
- Manual TLS connection testing documented in TLS_NIS2_README.md

## Review Checklist

- [ ] Code changes reviewed
- [ ] Relevant evidence provided
- [ ] Configuration changes documented
- [ ] Meets NIS2 encryption requirements for device-to-information-system traffic

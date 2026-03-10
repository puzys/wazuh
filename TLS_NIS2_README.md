# Wazuh Agent-Manager TLS Support (NIS2 Compliant)

This fork adds TLS/SSL encryption to the agent-manager data channel to satisfy NIS2 Lithuanian (and some other country specific NIS2 adaptations) directive requirements for device-to-information-system communication.

## Manager Configuration

Add a `<remote>` block with `connection` set to `secure_tls` in `ossec.conf` (or `wazuh-manager.conf`). Ensure you have built from this fork; upstream Wazuh does not support `secure_tls`.

**Important:** Remoted runs in a chroot (typically `/var/wazuh-manager`). TLS paths in the config are resolved **relative to the chroot root**, not the host filesystem. Use paths like `/etc/certs/...` so they resolve to `$INSTALL_DIR/etc/certs/...` inside the chroot.

```xml
<ossec_config>
  <remote>
    <connection>secure_tls</connection>
    <port>1516</port>
    <tls_certificate>/etc/certs/sslmanager.pem</tls_certificate>
    <tls_key>/etc/certs/sslmanager.key</tls_key>
    <tls_ca>/etc/certs/rootca.pem</tls_ca>
    <tls_ciphers>HIGH:!ADH:!EXP:!MD5:!RC4</tls_ciphers>
  </remote>
</ossec_config>
```

Place the actual cert files under `$INSTALL_DIR/etc/certs/` (e.g. `/var/wazuh-manager/etc/certs/`). Both `.pem` and `.cert` extensions work; `.pem` is more common in Wazuh.

### Configuration Options

| Option | Required | Description |
|--------|----------|-------------|
| `connection` | Yes | Must be `secure_tls` for TLS mode |
| `port` | No | Default: 1516 |
| `tls_certificate` | Yes | Path to server certificate (chroot-relative) |
| `tls_key` | Yes | Path to server private key (chroot-relative) |
| `tls_ca` | No | Path to CA certificate for client verification (chroot-relative) |
| `tls_ciphers` | No | OpenSSL cipher list (default: HIGH:!ADH:!EXP:!MD5:!RC4:!3DES:!CAMELLIA:@STRENGTH) |

You can use the same certificates as `wazuh-authd` (enrollment). Generate them, then copy into the chroot:

```bash
# Generate (outside chroot)
/var/wazuh-manager/bin/wazuh-authd -C 365 -B 2048 -K /tmp/sslmanager.key -X /tmp/sslmanager.pem -S "/C=LT/ST=Vilnius/CN=wazuh-manager/"

# Copy into chroot
mkdir -p /var/wazuh-manager/etc/certs
cp /tmp/sslmanager.pem /tmp/sslmanager.key /var/wazuh-manager/etc/certs/
cp /path/to/rootca.pem /var/wazuh-manager/etc/certs/
```

## Agent Configuration

### Native TLS

The agent supports native TLS for the manager connection. Add `<use_tls>yes</use_tls>` and optionally `<tls_port>1516</tls_port>` to your `<server>` block:

```xml
<ossec_config>
  <client>
    <server>
      <address>manager-ip</address>
      <use_tls>yes</use_tls>
      <tls_port>1516</tls_port>
      <tls_certificate_path>/var/ossec/etc/certs/agent.pem</tls_certificate_path>
      <tls_key_path>/var/ossec/etc/certs/agent.key</tls_key_path>
      <tls_ca_path>/var/ossec/etc/certs/rootca.pem</tls_ca_path>
    </server>
  </client>
</ossec_config>
```

When `use_tls` is yes, the agent connects to `tls_port` (1516); `<port>` and `<protocol>` are ignored for the TLS connection.

**Agent TLS options:**

| Option | Required | Description |
|--------|----------|-------------|
| `use_tls` | No | `yes` or `no` (default: no) |
| `tls_port` | No | Manager TLS port (default: 1516) |
| `tls_certificate_path` | When use_tls | Agent certificate (or use enrollment paths) |
| `tls_key_path` | When use_tls | Agent private key (or use enrollment paths) |
| `tls_ca_path` | When use_tls | CA certificate for server verification (or use enrollment paths) |

If `tls_certificate_path`, `tls_key_path`, and `tls_ca_path` are not set, the agent falls back to enrollment configuration (`agent_certificate_path`, `agent_key_path`, `server_ca_path`) when enrollment is configured.

## Certificate Management

### PKI Layout

- **CA certificate** – Signs manager and agent certs. Distribute to manager and all agents.
- **Manager certificate** – Server cert for the manager. CN should match manager hostname or use a SAN.
- **Agent certificate** – Client cert for each agent. Can be per-agent or shared (e.g. one cert per agent group).

### Generation (example)

```bash
# 1. Create CA (once)
openssl genrsa -out rootca.key 4096
openssl req -new -x509 -days 3650 -key rootca.key -out rootca.pem -subj "/C=LT/ST=Vilnius/O=MyOrg/CN=Wazuh-CA"

# 2. Manager cert
openssl genrsa -out sslmanager.key 2048
openssl req -new -key sslmanager.key -out sslmanager.csr -subj "/C=LT/ST=Vilnius/CN=wazuh-manager"
openssl x509 -req -in sslmanager.csr -CA rootca.pem -CAkey rootca.key -CAcreateserial -out sslmanager.pem -days 365

# 3. Agent cert (per agent or shared)
openssl genrsa -out agent.key 2048
openssl req -new -key agent.key -out agent.csr -subj "/C=LT/ST=Vilnius/CN=agent-001"
openssl x509 -req -in agent.csr -CA rootca.pem -CAkey rootca.key -CAcreateserial -out agent.pem -days 365
```

### Deployment

| Role   | Files to deploy                          | Notes |
|--------|------------------------------------------|-------|
| Manager| `rootca.pem`, `sslmanager.pem`, `sslmanager.key` | Place under `$INSTALL_DIR/etc/certs/`; config uses chroot-relative paths (e.g. `/etc/certs/`) |
| Agent  | `rootca.pem`, `agent.pem`, `agent.key`   | Agent does not chroot; use normal paths (e.g. `/var/ossec/etc/`) |

### Renewal

- Renew before expiry (e.g. 30 days). Restart manager/agent after replacing certs.
- Or use `wazuh-authd` for enrollment: it issues agent certs; use the same CA for manager TLS.

## Running Both Secure and Secure TLS

You can run both legacy (AES) and TLS listeners by defining two `<remote>` blocks:

```xml
<remote>
  <connection>secure</connection>
  <port>1514</port>
</remote>
<remote>
  <connection>secure_tls</connection>
  <port>1516</port>
  <tls_certificate>/etc/certs/sslmanager.pem</tls_certificate>
  <tls_key>/etc/certs/sslmanager.key</tls_key>
  <tls_ca>/etc/certs/rootca.pem</tls_ca>
</remote>
```

## Backward Compatibility (No Breaking Changes)

**Manager:**
- `secure_tls` is a new connection type. Existing `<remote><connection>secure</connection>` blocks are unchanged.
- Default port 1514 (AES) is unchanged. TLS uses port 1516 only when explicitly configured.
- You can run both: one `<remote>` for `secure` (1514) and one for `secure_tls` (1516).

**Agent:**
- `use_tls` defaults to `no`. Agents without `<use_tls>yes</use_tls>` behave exactly as before (plain TCP to port 1514).
- No config change = no behavior change.

**Summary:** Existing setups continue to work. TLS is opt-in.

## Version Compatibility

If you use the Wazuh indexer and dashboard from packages, keep the manager on the same major version. For example, if indexer/dashboard are 4.14.x, rebase this fork on the 4.14.3 tag so the full stack stays compatible. Wazuh 5.0 changed API endpoints and may not work with 4.x indexer/dashboard.

## Build

Build as standard Wazuh. The TLS changes are in:

**Manager:**
- `src/config/` – remote-config.c, remote-config.h
- `src/remoted/` – remoted.c, secure.c, netbuffer.c, remoted.h
- `src/headers/` – defs.h (DEFAULT_SECURE_TLS), ssl_op.h

**Agent:**
- `src/config/` – client-config.c, client-config.h (use_tls, tls_port, tls_*_path)
- `src/os_net/` – os_net.c, os_net.h (OS_ConnectTLS, TLS-aware OS_SendSecureTCP/OS_RecvSecureTCP)
- `src/client-agent/` – start_agent.c (TLS connection when use_tls)

Ensure OpenSSL is available (Wazuh already depends on it).

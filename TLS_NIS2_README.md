# Wazuh Agent-Manager TLS Support (NIS2 Compliant)

This fork adds TLS/SSL encryption to the agent-manager data channel to satisfy NIS2 Lithuanian (and some other country specific NIS2 adaptations) directive requirements for device-to-information-system communication.

## Manager Configuration

Add a `<remote>` block with `connection` set to `secure_tls` in `ossec.conf`:

```xml
<ossec_config>
  <remote>
    <connection>secure_tls</connection>
    <port>1516</port>
    <tls_certificate>/var/ossec/etc/sslmanager.cert</tls_certificate>
    <tls_key>/var/ossec/etc/sslmanager.key</tls_key>
    <tls_ca>/var/ossec/etc/rootca.cert</tls_ca>
    <tls_ciphers>HIGH:!ADH:!EXP:!MD5:!RC4</tls_ciphers>
  </remote>
</ossec_config>
```

### Configuration Options

| Option | Required | Description |
|--------|----------|-------------|
| `connection` | Yes | Must be `secure_tls` for TLS mode |
| `port` | No | Default: 1516 |
| `tls_certificate` | Yes | Path to server certificate |
| `tls_key` | Yes | Path to server private key |
| `tls_ca` | No | Path to CA certificate for client verification |
| `tls_ciphers` | No | OpenSSL cipher list (default: HIGH:!ADH:!EXP:!MD5:!RC4:!3DES:!CAMELLIA:@STRENGTH) |

You can use the same certificates as `wazuh-authd` (enrollment). Generate them with:

```bash
/var/ossec/bin/wazuh-authd -C 365 -B 2048 -K /var/ossec/etc/sslmanager.key -X /var/ossec/etc/sslmanager.cert -S "/C=LT/ST=Vilnius/CN=wazuh-manager/"
```

## Agent Connection Options

### Option A: Native TLS (Recommended for NIS2)

The agent supports native TLS for the manager connection. Add `<use_tls>yes</use_tls>` and optionally `<tls_port>1516</tls_port>` to your `<manager>` block:

```xml
<ossec_config>
  <client>
    <manager>
      <address>manager-ip</address>
      <port>1514</port>
      <use_tls>yes</use_tls>
      <tls_port>1516</tls_port>
      <tls_certificate_path>/var/ossec/etc/agent.cert</tls_certificate_path>
      <tls_key_path>/var/ossec/etc/agent.key</tls_key_path>
      <tls_ca_path>/var/ossec/etc/rootca.cert</tls_ca_path>
    </manager>
  </client>
</ossec_config>
```

**Agent TLS options:**

| Option | Required | Description |
|--------|----------|-------------|
| `use_tls` | No | `yes` or `no` (default: no) |
| `tls_port` | No | Manager TLS port (default: 1516) |
| `tls_certificate_path` | When use_tls | Agent certificate (or use enrollment paths) |
| `tls_key_path` | When use_tls | Agent private key (or use enrollment paths) |
| `tls_ca_path` | When use_tls | CA certificate for server verification (or use enrollment paths) |

If `tls_certificate_path`, `tls_key_path`, and `tls_ca_path` are not set, the agent falls back to enrollment configuration (`agent_certificate_path`, `agent_key_path`, `server_ca_path`) when enrollment is configured.

### Option B: TLS Proxy (stunnel)

Use stunnel if you prefer not to use native agent TLS. The agent connects to localhost; stunnel forwards to the manager over TLS.

**Manager:** Listen on port 1516 with `secure_tls`.

**Agent host:** Run stunnel:

```ini
; /etc/stunnel/agent-tls.conf
[agent-to-manager]
client = yes
accept = 127.0.0.1:1514
connect = manager-ip:1516
verifyChain = yes
CAfile = /path/to/ca.crt
```

**Agent ossec.conf:** Keep default (connects to 127.0.0.1:1514). Stunnel listens there and forwards to manager:1516 over TLS.

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
  <tls_certificate>/var/ossec/etc/sslmanager.cert</tls_certificate>
  <tls_key>/var/ossec/etc/sslmanager.key</tls_key>
</remote>
```

## Backward Compatibility

- The manager can run both `secure` (port 1514, AES) and `secure_tls` (port 1516, TLS) by defining two `<remote>` blocks.
- Existing agents continue using port 1514.
- New NIS2-compliant deployments use port 1516 with TLS.

## Build

Build as standard Wazuh. The TLS changes are in:

**Manager:**
- `src/config/` – remote-config, remote-config.h
- `src/remoted/` – remoted.c, secure.c, netbuffer.c, remoted.h
- `src/shared/include/defs.h` – DEFAULT_SECURE_TLS

**Agent:**
- `src/config/` – client-config.c, client-config.h (use_tls, tls_port, tls_*_path)
- `src/shared/os_net/` – os_net.c, os_net.h (OS_ConnectTLS, TLS-aware OS_SendSecureTCP/OS_RecvSecureTCP)
- `src/client-agent/` – start_agent.c (TLS connection when use_tls)

Ensure OpenSSL is available (Wazuh already depends on it).

/* Copyright (C) 2015, Wazuh Inc.
 * SSL/TLS operations for agent-manager channel (NIS2 compliant)
 */

#ifndef SSL_OP_H
#define SSL_OP_H

#include <openssl/ssl.h>
#include <openssl/err.h>

#define DEFAULT_CIPHERS "HIGH:!ADH:!EXP:!MD5:!RC4:!3DES:!CAMELLIA:@STRENGTH"

SSL_CTX *os_ssl_keys(int is_server, const char *os_dir, const char *ciphers,
                     const char *cert, const char *key, const char *ca_cert,
                     int auto_method);

#endif /* SSL_OP_H */

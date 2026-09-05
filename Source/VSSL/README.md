# VSSL

Voyager SSL plugin (`PROGDIR:Plugins/voyager_ssl.vlib`). The browser talks only to the `VSSL_*` LVOs in `fd/vssl.fd` (ABI v9). Crypto backends are separate plugins that export that same jump table.

| Directory | Backend | Notes |
| --- | --- | --- |
| `amitls/` | [amitls.library](../../../AmiTLS) (BearSSL) | Current OS3 plugin. `smake` here. |
| `openssl/` | OpenSSL 0.9.2b | Historical; do not ship. |
| *(later)* | AmiSSL | Same `VSSL_*` ABI, different `.vlib` build. |

Runtime for the AmiTLS plugin: `LIBS:amitls.library` (v1, revision 2+ for `TlsHandshake`) and a PEM CA bundle (`PROGDIR:Certificates/cacert.pem` or the other paths listed in `amitls/vssl.c`).

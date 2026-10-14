# VSSL

Voyager SSL plugin (`PROGDIR:Plugins/voyager_ssl.vlib`). The browser talks only to the `VSSL_*` LVOs in `fd/vssl.fd` (ABI v9). Crypto backends are separate plugins that export that same jump table.

| Directory | Backend | Notes |
| --- | --- | --- |
| `amissl/` | [AmiSSL](../../../../AmiSSL-5.27-SDK) 5.27 (OpenSSL 3.6) | Preferred when installed. `smake` here produces `voyager_ssl_amissl.vlib`. |
| `amitls/` | [amitls.library](../../../AmiTLS) (BearSSL) | Fallback plugin `voyager_ssl.vlib`. `smake` here. |
| `openssl/` | OpenSSL 0.9.2b | Historical; do not ship. |

Runtime for the AmiSSL plugin: `LIBS:amisslmaster.library` (v5+) opening AmiSSL 5.27 (`AMISSL_V362`). The CA bundle is `PROGDIR:Certificates/cacert.pem`. If this plugin's libinit cannot open AmiSSL, Voyager opens the AmiTLS plugin instead.

Runtime for the AmiTLS plugin: `LIBS:amitls.library` (v1, revision 2+ for `TlsHandshake`) and the same CA bundle, `PROGDIR:Certificates/cacert.pem`.

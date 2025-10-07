# TLS 1.3

A replica of the TLS 1.3 protocol roughly following [RFC 8446](https://datatracker.ietf.org/doc/html/rfc8446).

It implements cryptographic algorithms such as:
- AES-256
- ECDH over Curve25519
- SHA-256
- RSA-2048
- GCM

From first principles and **without** libraries such as OpenSSL.

To manipulate the large numbers used in these algorithms, it uses a custom big number library.

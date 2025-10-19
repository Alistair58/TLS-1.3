#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdint.h>

typedef enum Groups{
    /* Elliptic Curve Groups (ECDHE) */
    secp256r = 0x0017,
    secp384r1 = 0x0018,
    secp521r1 = 0x0019,
    x25519 = 0x001D,
    x448 = 0x001E,

    /* Finite Field Groups (DHE) */
    ffdhe2048 = 0x0100,
    ffdhe3072 = 0x0101,
    ffdhe4096 = 0x0102,
    ffdhe6144 = 0x0103,
    ffdhe8192 = 0x0104
} Groups;

typedef enum Ciphers{
    TLS_AES_128_GCM_SHA256 = 0x01,
    TLS_AES_256_GCM_SHA384 = 0x02,
    TLS_CHACHA20_POLY1305_SHA256 = 0x03,
    TLS_AES_128_CCM_SHA256 = 0x04,
    TLS_AES_128_CCM_8_SHA256 = 0x05
} Ciphers;

typedef enum SignatureScheme {
    /* RSASSA-PKCS1-v1_5 algorithms */
    rsa_pkcs1_sha256 = 0x0401,
    rsa_pkcs1_sha384 = 0x0501,
    rsa_pkcs1_sha512 = 0x0601,

    /* ECDSA algorithms */
    ecdsa_secp256r1_sha256 = 0x0403,
    ecdsa_secp384r1_sha384 = 0x0503,
    ecdsa_secp521r1_sha512 = 0x0603,

    /* RSASSA-PSS algorithms with public key OID rsaEncryption */
    rsa_pss_rsae_sha256 = 0x0804,
    rsa_pss_rsae_sha384 = 0x0805,
    rsa_pss_rsae_sha512 = 0x0806,

    /* EdDSA algorithms */
    ed25519 = 0x0807,
    ed448 = 0x0808,

    /* RSASSA-PSS algorithms with public key OID RSASSA-PSS */
    rsa_pss_pss_sha256 = 0x0809,
    rsa_pss_pss_sha384 = 0x080a,
    rsa_pss_pss_sha512 = 0x080b,

    /* Legacy algorithms */
    rsa_pkcs1_sha1 = 0x0201,
    ecdsa_sha1 = 0x0203
} SignatureScheme;

typedef struct ClientHello{
    uint32_t clientRandom;
    int cipherSuites[5][2]; // TLS 1.3 only supports 5 cipher suites
    int supportedGroups[10];
    int signatureAlgorithms[16];
    uint32_t keyExchange[8];
} ClientHello;

typedef struct Certificate{
    char CA[50];
    uint32_t publicKey[8];
    uint32_t CASignature[8];
    uint32_t serverSignature[8];
} Certificate;

typedef struct ServerHello{
    uint32_t serverRandom;
    int cipherSuite[2]; // TLS 1.3 only supports 5 cipher suites
    int curveGroup;
    int signatureAlgorithm;
    Certificate certificate; 
    uint32_t keyExchange[8];
    uint32_t MAC[8]; //Sort of server finished
} ServerHello;



typedef struct CurveGroupParams{
    uint32_t p[8];
    uint32_t a[8];
    uint32_t b[8];
    uint32_t G[2][8];
    uint32_t n[8];
} CurveGroupParams;


extern CurveGroupParams secp256rParams;
extern CurveGroupParams curve25519Params;

#endif
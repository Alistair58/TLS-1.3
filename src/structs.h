#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdint.h>

enum{
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
} groups;

enum{
    TLS_AES_128_GCM_SHA256 = 0x01,
    TLS_AES_256_GCM_SHA384 = 0x02,
    TLS_CHACHA20_POLY1305_SHA256 = 0x03,
    TLS_AES_128_CCM_SHA256 = 0x04,
    TLS_AES_128_CCM_8_SHA256 = 0x05
} ciphers;

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



struct CurveGroupParams{
    uint32_t p[8];
    uint32_t a[8];
    uint32_t b[8];
    uint32_t G[2][8];
    uint32_t n[8];
}
secp256rParams = {
    {0xffffffffUL,0x00000001UL,0x00000000UL,0x00000000UL,0x00000000UL,0xffffffffUL,0xffffffffUL,0xffffffffUL},
    {0xffffffffUL,0x00000001UL,0x00000000UL,0x00000000UL,0x00000000UL,0xffffffffUL,0xffffffffUL,0xfffffffcUL},
    {0x5ac635d8UL,0xaa3a93e7UL,0xb3ebbd55UL,0x769886bcUL,0x651d06b0UL,0xcc53b0f6UL,0x3bce3c3eUL,0x27d2604bUL},
    {{0x6b17d1f2UL,0xe12c4247UL,0xf8bce6e5UL,0x63a440f2UL,0x77037d81UL,0x2deb33a0UL,0xf4a13945UL,0xd898c296UL}, 
    {0x4fe342e2UL,0xfe1a7f9bUL,0x8ee7eb4aUL,0x7c0f9e16UL,0x2bce3357UL,0x6b315eceUL,0xcbb64068UL,0x37bf51f5UL}},
    {0xffffffffUL,0x00000000UL,0xffffffffUL,0xffffffffUL,0xbce6faadUL,0xa7179e84UL,0xf3b9cac2UL,0xfc632551UL}
}, 
curve25519Params = {
    {0x7FFFFFFFUL,0xFFFFFFFFUL,0xFFFFFFFFUL,0xFFFFFFFFUL,0xFFFFFFFFUL,0xFFFFFFFFUL,0xFFFFFFFFUL,0xFFFFFFEDUL},
    {0,         0,         0,         0,         0,         0,         0,         486662    },
    {0,         0,         0,         0,         0,         0,         0,         0         },
    {{0,         0,         0,         0,         0,         0,         0,         9         },
    {0x20AE19A1UL,0xB8A086B4UL,0xE01EDD2CUL,0x7748D14CUL,0x923D4D7EUL,0x6D7C61B2UL,0x29E9C5A2UL,0x7ECED3D9UL}
    },
    {0x10000000UL,      0,         0,         0,         0x14def9deUL,0xa2f79cd6UL,0x5812631aUL,0x5cf5d3edUL } //G has order 2^252 + 0x14def9deULa2f79cd65812631a5cf5d3ed
    //Cofactor 8
};
typedef struct CurveGroupParams CurveGroupParams;

#endif
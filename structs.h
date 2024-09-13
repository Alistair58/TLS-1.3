enum
{
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

enum
{
    TLS_AES_128_GCM_SHA256 = 0x01,
    TLS_AES_256_GCM_SHA384 = 0x02,
    TLS_CHACHA20_POLY1305_SHA256 = 0x03,
    TLS_AES_128_CCM_SHA256 = 0x04,
    TLS_AES_128_CCM_8_SHA256 = 0x05
} ciphers;

enum
{
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

struct ClientHello
{
    unsigned long clientRandom;
    int cipherSuites[5][2]; // TLS 1.3 only supports 5 cipher suites
    int supportedGroups[10];
};

struct CurveGroupParams{
    unsigned long p[8];
    unsigned long a[8];
    unsigned long b[8];
    unsigned long G[2][8];
    unsigned long n[8];
}
secp256rParams = {
    {0xffffffff,0x00000001,0x00000000,0x00000000,0x00000000,0xffffffff,0xffffffff,0xffffffff},
    {0xffffffff,0x00000001,0x00000000,0x00000000,0x00000000,0xffffffff,0xffffffff,0xfffffffc},
    {0x5ac635d8,0xaa3a93e7,0xb3ebbd55,0x769886bc,0x651d06b0,0xcc53b0f6,0x3bce3c3e,0x27d2604b},
    {{0x6b17d1f2,0xe12c4247,0xf8bce6e5,0x63a440f2,0x77037d81,0x2deb33a0,0xf4a13945,0xd898c296}, 
    {0x4fe342e2,0xfe1a7f9b,0x8ee7eb4a,0x7c0f9e16,0x2bce3357,0x6b315ece,0xcbb64068,0x37bf51f5}},
    {0xffffffff,0x00000000,0xffffffff,0xffffffff,0xbce6faad,0xa7179e84,0xf3b9cac2,0xfc632551}
}, 
curve25519Params = {
    {0x7FFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFED},
    {0,         0,         0,         0,         0,         0,         0,         486662    },
    {0,         0,         0,         0,         0,         0,         0,         0         },
    {{0,         0,         0,         0,         0,         0,         0,         9         },
    {0x20AE19A1,0xB8A086B4,0xE01EDD2C,0x7748D14C,0x923D4D7E,0x6D7C61B2,0x29E9C5A2,0x7ECED3D9}
    },
    {0x10,      0,         0,         0,         0x14def9de,0xa2f79cd6,0x5812631a,0x5cf5d3ed } //G has order 2^252 + 0x14def9dea2f79cd65812631a5cf5d3ed
    //Cofactor 8
};
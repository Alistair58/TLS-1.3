#include "structs.h"

CurveGroupParams secp256rParams = {
    {0xffffffffUL,0x00000001UL,0x00000000UL,0x00000000UL,0x00000000UL,0xffffffffUL,0xffffffffUL,0xffffffffUL},
    {0xffffffffUL,0x00000001UL,0x00000000UL,0x00000000UL,0x00000000UL,0xffffffffUL,0xffffffffUL,0xfffffffcUL},
    {0x5ac635d8UL,0xaa3a93e7UL,0xb3ebbd55UL,0x769886bcUL,0x651d06b0UL,0xcc53b0f6UL,0x3bce3c3eUL,0x27d2604bUL},
    {{0x6b17d1f2UL,0xe12c4247UL,0xf8bce6e5UL,0x63a440f2UL,0x77037d81UL,0x2deb33a0UL,0xf4a13945UL,0xd898c296UL}, 
    {0x4fe342e2UL,0xfe1a7f9bUL,0x8ee7eb4aUL,0x7c0f9e16UL,0x2bce3357UL,0x6b315eceUL,0xcbb64068UL,0x37bf51f5UL}},
    {0xffffffffUL,0x00000000UL,0xffffffffUL,0xffffffffUL,0xbce6faadUL,0xa7179e84UL,0xf3b9cac2UL,0xfc632551UL}
};

CurveGroupParams curve25519Params = {
    {0x7FFFFFFFUL,0xFFFFFFFFUL,0xFFFFFFFFUL,0xFFFFFFFFUL,0xFFFFFFFFUL,0xFFFFFFFFUL,0xFFFFFFFFUL,0xFFFFFFEDUL},
    {0,         0,         0,         0,         0,         0,         0,         486662    },
    {0,         0,         0,         0,         0,         0,         0,         0         },
    {{0,         0,         0,         0,         0,         0,         0,         9         },
    {0x20AE19A1UL,0xB8A086B4UL,0xE01EDD2CUL,0x7748D14CUL,0x923D4D7EUL,0x6D7C61B2UL,0x29E9C5A2UL,0x7ECED3D9UL}
    },
    {0x10000000UL,      0,         0,         0,         0x14def9deUL,0xa2f79cd6UL,0x5812631aUL,0x5cf5d3edUL } //G has order 2^252 + 0x14def9deULa2f79cd65812631a5cf5d3ed
    //Cofactor 8
};
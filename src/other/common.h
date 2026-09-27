#ifndef COMMON_H
#define COMMON_H

#include "../other/der.h"

typedef struct sockaddr_in sockaddr_in; 

typedef struct ClientHello{
    uint32_t clientRandom;
    int cipherSuites[5][2]; // TLS 1.3 only supports 5 cipher suites
    int supportedGroups[10];
    int signatureAlgorithms[16];
    bignum keyExchange;
} ClientHello;

extern StructInfo clientHelloInfo;

typedef struct ServerHello{
    uint32_t serverRandom;
    int cipherSuite[2]; // TLS 1.3 only supports 5 cipher suites
    int curveGroup;
    int signatureAlgorithm;
    uchar *certificate;
    bignum keyExchange;
    bignum MAC; //Sort of server finished
} ServerHello;

extern StructInfo serverHelloInfo;

#endif
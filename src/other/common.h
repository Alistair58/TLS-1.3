#ifndef SHARED_H
#define SHARED_H

#include "../pki/x509.h"

typedef unsigned char uchar;
typedef struct in_addr{
    //TODO
} in_addr;


typedef struct ClientHello{
    uint32_t clientRandom;
    int cipherSuites[5][2]; // TLS 1.3 only supports 5 cipher suites
    int supportedGroups[10];
    int signatureAlgorithms[16];
    uint32_t keyExchange[8];
} ClientHello;

typedef struct ServerHello{
    uint32_t serverRandom;
    int cipherSuite[2]; // TLS 1.3 only supports 5 cipher suites
    int curveGroup;
    int signatureAlgorithm;
    asn1Certificate certificate; 
    uint32_t keyExchange[8];
    uint32_t MAC[8]; //Sort of server finished
} ServerHello;


#endif
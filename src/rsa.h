#ifndef RSA_H
#define RSA_H


#include <stdbool.h>
#include "globals.h"
#include "bigmaths.h"

//RSASSA-PKCS1-v1_5 (SHA 256)
//The official RFC (3447) doesn't specify anything specific
//It is just RSA 

//Used for the CA to sign the certificate
//And the server to sign the handshake
//The private key is hard coded into the server program
//The public key is displayed on the certificate

//Using 2048 bit keys (n is 2048)
//Which is an array of 64 uint32_ts
//Therefore p and q will be 1028 bits(32 length array)
typedef struct RSAPublicKey{
    bignum n;
    int lenN;
    uint32_t e;
} RSAPublicKey;

typedef struct RSAPrivateKey{
    bignum p;
    int lenP;
    bignum q;
    int lenQ;
} RSAPrivateKey;

typedef struct RSAKeyPair{
    RSAPrivateKey privateKey;
    RSAPublicKey publicKey;
} RSAKeyPair;


bool isPrime(bignum n,int lenN);
bignum encryptRSA(uchar *msg,int lenMsg,RSAKeyPair kp);
uchar *decryptRSA(bignum encryptedMessage,int lenEM,RSAKeyPair kp);
RSAKeyPair generateKeys(int numBits);

void extendedEuclidean(uint32_t exp,bignum totient,int lenTotient,bignum dest,int lenDest);

#endif
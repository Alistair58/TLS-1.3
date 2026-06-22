#ifndef X509_H
#define X509_H


#include <stdint.h>
#include "rsa.h"
#include "globals.h"
#include "structs.h"
#include "bigmaths.h"


typedef enum TBSVersion{
    v3 = 2, //RFC 5280
} TBSVersion;

typedef struct asn1Validity{
    //YYMMDDHHMMSSZ - RFC 5280
    //Null terminated
    uchar notBefore[14];
    uchar notAfter[14];
}asn1Validity;

typedef struct asn1SubjectPublicKeyInfo{
    SignatureScheme algorithm;
    RSAPublicKey subjectPublicKey;
}asn1SubjectPublicKeyInfo;

// TBS - to be signed
typedef struct asn1TBSCertificate{
    TBSVersion version;
    int serialNumber;
    SignatureScheme signature;
    uchar issuer[50];
    asn1Validity validity;
    uchar subject[50];
    asn1SubjectPublicKeyInfo subjectPublicKeyInfo;
}asn1TBSCertificate;


//ASN1 - Abstract Structure Notation 1
typedef struct asn1Certificate{
    asn1TBSCertificate tbsCertif;
    SignatureScheme signatureAlgorithm;
    bignum signatureValue;
    int lenSignatureValue;
}asn1Certificate;



typedef uint8_t certifStatus;
#define VALID 0
#define OUT_OF_DATE 1
#define INVALID_SIGNATURE 2

/**
 * Create an unsigned X509 certificate with the subject's info and save it under fname
 */
void signX509(RSAKeyPair issuerKp,uchar *issuer,uchar *fname);

/**
 * Given an unsigned X509 certificate at fname, sign it with the issuer's private key and save it back to fname
 */
void generateX509(RSAPublicKey subjectPk,uchar *subject,uchar *fname);

/**
 * Given an X509 certificate at fname, check that its signature and date are valid
 */
certifStatus checkX509(RSAPublicKey issuerPk,uchar *fname);

#endif
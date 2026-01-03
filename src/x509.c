#include "x509.h"
#include <stdio.h>
#include <string.h>
#include "sha.h"

//An implementation of a X509 certificate creation roughly following RFC 5280


#define DER_SEQUENCE 0x30
#define DER_INTEGER 0x02
#define DER_UTF8STRING 0x0C
#define DER_BITSTRING 0x03

#define RSA_NUM_BITS 1024


static asn1Certificate generateAsn1X509(RSAKeyPair kp);
static DER asn1TBSToDER(asn1TBSCertificate asn1TBSCertif);
static int derEncodeBignum(uchar *result,bignum n,int lenN);
static int derEncodeString(uchar *result,uchar *string,int lenString);
static int derEncodeInt(uchar *result,int num);
static DER asn1ToDER(asn1Certificate asn1Certif);
static Base64 base64Encode(uchar *data,int lenData);

void generateX509(RSAKeyPair kp){
    asn1Certificate asn1Certif = generateAsn1X509(kp);
    DER derCertif = asn1ToDER(asn1Certif);
    Base64 b64Certif = base64Encode(derCertif.data,derCertif.lenData);
    uchar pemTemplate[] = "-----BEGIN CERTIFICATE-----%s\n-----END CERTIFICATE-----\n\n";
    FILE *fhand;
    fhand = fopen("certif.pem","w");
    fprintf(fhand,pemTemplate,b64Certif.data);
    fclose(fhand);

    free(derCertif.data);
    free(b64Certif.data);
}


static asn1Certificate generateAsn1X509(RSAKeyPair kp){
    asn1Certificate certif;
    certif.tbsCertif.version = v3;
    certif.tbsCertif.serialNumber = 0;
    certif.tbsCertif.signature = rsa_pkcs1_sha256;
    uchar issuer[] = "me";
    memcpy(certif.tbsCertif.issuer,issuer,sizeof(issuer)); 
    //TODO add timings
    uchar notBefore[] = "YYMMDDHHMMSSZ";
    uchar notAfter[] = "YYMMDDHHMMSSZ";
    memcpy(certif.tbsCertif.validity.notBefore,notBefore,sizeof(notBefore));
    memcpy(certif.tbsCertif.validity.notAfter,notAfter,sizeof(notAfter));
    uchar subject[] = "me";
    memcpy(certif.tbsCertif.subject,subject,sizeof(subject));
    certif.tbsCertif.subjectPublicKeyInfo.algorithm = rsa_pkcs1_sha256;
    certif.tbsCertif.subjectPublicKeyInfo.subjectPublicKey = kp.publicKey;
    certif.signatureAlgorithm = rsa_pkcs1_sha256;

    DER der = asn1TBSToDER(certif.tbsCertif);
    bignum hash = sha256(der.data,der.lenData);
    //Not decrypting, just using the private key to encrypt
    uchar *signatureString = calloc(kp.publicKey.lenN*sizeof(uint32_t),sizeof(uchar));
    decryptRSA(hash,8,kp,signatureString,kp.publicKey.lenN*sizeof(uint32_t));
    certif.signatureValue = calloc(RSA_NUM_BITS/32,sizeof(uint32_t));
    if(!certif.signatureValue){
        allocError();
    }
    memcpy(certif.signatureValue,signatureString,RSA_NUM_BITS/8);
    certif.lenSignatureValue = RSA_NUM_BITS/32;
    free(signatureString);
    free(hash);
    free(der.data);
}

static DER asn1TBSToDER(asn1TBSCertificate asn1TBSCertif){
    DER result;
    //Allocate a buffer that is sufficient in size
    //We will resize when we know the length
    result.data = (uchar*) malloc(2048);
    if(!result.data){
        allocError();
    }
    int index = 0;
    
    //TBS certif sequence
    result.data[index++] = DER_SEQUENCE;
    int tbsSequenceLengthIndex = index;
    index++; //reserve the index
    index += derEncodeInt(&result.data[index],asn1TBSCertif.version);
    index += derEncodeInt(&result.data[index],asn1TBSCertif.serialNumber);
    index += derEncodeInt(&result.data[index],asn1TBSCertif.signature);
    index += derEncodeString(&result.data[index],asn1TBSCertif.issuer,sizeof(asn1TBSCertif.issuer));
    

    //Vaildity
    result.data[index++] = DER_SEQUENCE;
    int validitySequenceLengthIndex = index;
    index++;
    index += derEncodeString(&result.data[index],asn1TBSCertif.validity.notBefore,sizeof(asn1TBSCertif.validity.notBefore));
    index += derEncodeString(&result.data[index],asn1TBSCertif.validity.notAfter,sizeof(asn1TBSCertif.validity.notAfter));
    index += derEncodeString(&result.data[index],asn1TBSCertif.subject,sizeof(asn1TBSCertif.subject));
    result.data[validitySequenceLengthIndex] = index-(validitySequenceLengthIndex+1);

    //Public key info
    result.data[index++] = DER_SEQUENCE;
    int keyInfoSequenceLengthIndex = index;
    index++;
    index += derEncodeInt(&result.data[index],asn1TBSCertif.subjectPublicKeyInfo.algorithm);
    result.data[index++] = DER_SEQUENCE;
    index += derEncodeInt(&result.data[index],asn1TBSCertif.subjectPublicKeyInfo.subjectPublicKey.e);
    index += derEncodeBignum(&result.data[index],asn1TBSCertif.subjectPublicKeyInfo.subjectPublicKey.n,asn1TBSCertif.subjectPublicKeyInfo.subjectPublicKey.lenN);
    result.data[keyInfoSequenceLengthIndex] = index-(keyInfoSequenceLengthIndex+1);

    result.data[tbsSequenceLengthIndex] = index-(tbsSequenceLengthIndex+1);
    uchar *resizedResult = (uchar*) malloc(index);
    free(result.data);
    result.data = resizedResult;
    result.lenData = index;
    return result;
}

static int derEncodeBignum(uchar *result,bignum n,int lenN){
    result[0] = DER_BITSTRING;
    result[1] = lenN*4;
    for(int i=0;i<lenN*4;i++){
        //big endian
        result[i+2] = (n[i/4] >> (3-(i&0x3))) & 0xff;
    }
    return lenN*4+2;
}


static int derEncodeString(uchar *result,uchar *string,int lenString){
    result[0] = DER_UTF8STRING;
    result[1] = lenString;
    for(int i=0;i<lenString;i++){
        result[i+2] = string[i];
    }
    return lenString+2;
}

static int derEncodeInt(uchar *result,int num){
    result[0] = DER_INTEGER;
    result[1] = 4;
    for(int i=0;i<4;i++){
        //big endian
        result[2+i] = (num >> ((3-i)*8)) & 0xff;
    }
    return 6;
}

static DER asn1ToDER(asn1Certificate asn1Certif){
    DER result;
    //Allocate a buffer that is sufficient in size
    //We will resize when we know the length
    result.data = (uchar*) malloc(2048);
    if(!result.data){
        allocError();
    }
    int index = 0;
    
    //Certif sequence
    result.data[index++] = DER_SEQUENCE;
    int certifSequenceLengthIndex = index;
    index++; //reserve the index

    //tbsCertif
    DER tbsDER = asn1TBSToDER(asn1Certif.tbsCertif);
    memcpy(&result.data[index],tbsDER.data,tbsDER.lenData);
    index += tbsDER.lenData;
    free(tbsDER.data);

    //Signature Algorithm
    index += derEncodeInt(&result.data[index],asn1Certif.signatureAlgorithm);

    //Signature value
    index += derEncodeBignum(&result.data[index],asn1Certif.signatureValue,asn1Certif.lenSignatureValue*sizeof(uint32_t));

    //Signature length
    index += derEncodeInt(&result.data[index],asn1Certif.lenSignatureValue);

    result.data[certifSequenceLengthIndex] = index-(certifSequenceLengthIndex+1);

    uchar *resizedResult = (uchar*) malloc(index);
    free(result.data);
    result.data = resizedResult;
    result.lenData = index;
    return result;
}

static Base64 base64Encode(uchar *data,int lenData){
    Base64 result;
    int lenBits = lenData*8;
    int lenPaddingBits = lenBits%6;
    int len6BitChunksBits = lenBits+lenPaddingBits;
    int num6BitChunks = len6BitChunksBits / 6;
    //add a '=' for each 2 padding bits
    int lenPaddingChars = lenPaddingBits/2;
    int lenResultBytes = num6BitChunks/8 + lenPaddingChars;

    result.data = calloc(lenResultBytes,1);
    result.lenData = lenResultBytes;

    for(int i=0;i<num6BitChunks;i++){
        int inputIndex = i*6/8;
        uchar inputChunk0 = data[inputIndex];
        uchar inputChunk1 = (inputIndex>=lenData)?0:data[inputIndex+1];

        // i=0 -> chunk0 & 0b111111
        // i=1 -> chunk0 & 0b11  | chunk1 & 0b11110000 
        // i=2 -> chunk0 & 0b1111 | chunk1 & 0b11000000
        // i=3 -> chunk0 & 0b111111

        //There might be a nicer way of doing this
        uint8_t mask0,mask1;
        uint8_t chunk0Shift;
        switch(i%3){
            case 0:
                mask0 = 0b111111;
                mask1 = 0b0;
                chunk0Shift = 0;
                break;
            case 1:
                mask0 = 0b11;
                mask1 = 0b11110000;
                chunk0Shift = 4;
                break;
            case 2:
                mask0 = 0b1111;
                mask1 = 0b11000000;
                chunk0Shift = 2;
        }
        uint8_t chunk6Bits = ((inputChunk0 & mask0) << chunk0Shift) | inputChunk1 & mask1; 
        if(chunk6Bits<26){
            result.data[i] = 'A'+chunk6Bits;
        }
        else if(chunk6Bits<52){
            result.data[i] = 'a'+(chunk6Bits-26);
        }
        else if(chunk6Bits<62){
            result.data[i] = '0'+(chunk6Bits-52);
        }
        else if(chunk6Bits==62) result.data[i] = '+';
        else if(chunk6Bits==63) result.data[i] = '/';
    }
    //Padding
    for(int i=0;i<lenPaddingChars;i++){
        result.data[num6BitChunks+i] = '=';
    }

    return result;
}
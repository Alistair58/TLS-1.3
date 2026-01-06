#include "x509.h"
#include <stdio.h>
#include <string.h>
#include "sha.h"

//An implementation of a X509 certificate creation roughly following RFC 5280


#define DER_SEQUENCE 0x30
#define DER_INTEGER 0x02
#define DER_UTF8STRING 0x0C
#define DER_BITSTRING 0x03


static asn1Certificate generateAsn1X509(RSAPublickKey subjectPk,RSAKeyPair issuerKp);
static DER asn1TBSToDER(asn1TBSCertificate asn1TBSCertif);
static int derEncodeBignum(uchar *result,bignum n,int lenN);
static int derEncodeString(uchar *result,uchar *string,int lenString);
static int derEncodeInt(uchar *result,int num);
static DER asn1ToDER(asn1Certificate asn1Certif);
static Base64 base64Encode(uchar *data,int lenData);
static DER base64Decode(uchar *input,int lenInput);
static asn1Certificate DERToAsn1(DER der);

void generateX509(RSAPublickKey subjectPk,RSAKeyPair issuerKp,uchar *fname){
    asn1Certificate asn1Certif = generateAsn1X509(subjectPk,issuerKp);
    DER derCertif = asn1ToDER(asn1Certif);
    Base64 b64Certif = base64Encode(derCertif.data,derCertif.lenData);
    uchar pemTemplate[] = "-----BEGIN CERTIFICATE-----%s\n-----END CERTIFICATE-----\n\n";
    FILE *fhand = fopen(fname,"w");
    fprintf(fhand,pemTemplate,b64Certif.data);
    fclose(fhand);

    free(derCertif.data);
    free(b64Certif.data);
}


static asn1Certificate generateAsn1X509(RSAPublickKey subjectPk,RSAKeyPair issuerKp){
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
    certif.tbsCertif.subjectPublicKeyInfo.subjectPublicKey = subjectPk;
    certif.signatureAlgorithm = rsa_pkcs1_sha256;

    DER der = asn1TBSToDER(certif.tbsCertif);
    bignum hash = sha256(der.data,der.lenData);
    //Not decrypting, just using the private key to encrypt
    uchar *signatureString = calloc(issuerKp.publicKey.lenN*sizeof(uint32_t),sizeof(uchar));
    decryptRSA(hash,8,kp,signatureString,issuerKp.publicKey.lenN*sizeof(uint32_t));
    certif.signatureValue = calloc(issuerKp.publicKey.lenN,sizeof(uint32_t));
    if(!certif.signatureValue){
        allocError();
    }
    memcpy(certif.signatureValue,signatureString,issuerKp.publicKey.lenN*sizeof(uint32_t));
    certif.lenSignatureValue = issuerKp.publicKey.lenN;
    free(signatureString);
    free(hash);
    free(der.data);
    return certif;
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

Base64 base64Encode(uchar *data,int lenData){
    Base64 result;
    int lenBits = lenData*8;
    int lenPaddingBits = lenBits%6;
    int len6BitChunksBits = lenBits+lenPaddingBits;
    int num6BitChunks = len6BitChunksBits / 6;
    //add a '=' for each 2 padding bits
    int lenPaddingChars = lenPaddingBits/2;
    int lenResultBytes = num6BitChunks + lenPaddingChars;

    result.data = calloc(lenResultBytes,sizeof(uchar));
    result.lenData = lenResultBytes;

    for(int i=0;i<num6BitChunks;i++){
        int inputIndex = i*6/8;
        uchar inputChunk0 = data[inputIndex];
        uchar inputChunk1 = (inputIndex>=lenData)?0:data[inputIndex+1];

        // i=0 -> input[0] & 0b11111100 >> 2 | input[1] & 0b00000000 >> 2
        // i=1 -> input[0] & 0b00000011 << 4 | input[1] & 0b11110000 >> 4
        // i=2 -> input[1] & 0b00001111 << 2 | input[2] & 0b11000000 >> 6
        // i=3 -> input[2] & 0b00111111 << 0 | input[3] & 0b00000000 >> 8
        // i=4 -> input[3] & 0b11111100 >> 2 | input[4] & 0b00000000 >> 2
        // i=5 -> input[3] & 0b00000011 | input[4] & 0b11110000 

        //There might be a nicer way of doing this
        //It's sort of a pattern but not a very nice one
        
        uint8_t chunk6Bits;
        switch(i%4){
            case 0:
                chunk6Bits = (inputChunk0 & 0b11111100) >> 2;
                break;
            case 1:
                chunk6Bits = (inputChunk0 & 0b00000011) << 4 | (inputChunk1 & 0b11110000) >> 4;
                break;
            case 2:
                chunk6Bits = (inputChunk0 & 0b00001111) << 2 | (inputChunk1 & 0b11000000) >> 6;
                break;
            case 3:
                chunk6Bits = (inputChunk0 & 0b00111111);
                break;
        }
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

asn1Certificate X509ToAsn1(uchar *fname){
    FILE *fhand = fopen(fname,"r");
    const int lenBuff = 2000;
    uchar buff[2000] = {0};
    fgets(buff,lenBuff,fhand);
    DER derCertif = base64Decode(buff,lenBuff);
    asn1Certificate asn1Certif = DERToAsn1(derCertif);
    free(derCertif.data);
    return asn1Cerif;
}


certifStatus checkX509(RSAPublickKey issuerPk,uchar *fname){
    asn1Certificate asn1Cerif =  X509ToAsn1(fname);
    //TODO
    //Check signature 
    //Check date
}

static DER base64Decode(uchar *input,int lenInput){
    DER result;
    int lenBits = lenData*8;
    //Calculate the real length of the message
    int lenPaddingBits = 0;
    for(int i=lenInput-1;i>=0;i--){
        if(input[i]=='='){
            lenPaddingBits+=2;
            //8 for the '=' and 2 for the actual padding
            lenBits-=10;
        }
        else break;
    }
    
    int lenResultBytes = (lenBits/8)*6/8;

    result.data = calloc(lenResultBytes,sizeof(uchar));
    result.lenData = lenResultBytes;

    //TODO 

    return result;

}


static asn1Certificate DERToAsn1(DER der){
    //TODO
}
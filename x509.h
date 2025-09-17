//TODO remove
#include<structs.h>


typedef struct asn1Certificate{
    asn1TBSCertificate tbsCertif;
    SignatureScheme  signatureAlgorithm;
    bignum signatureValue;
    int lenSignatureValue;
}asn1Certificate;


typedef enum TBSVersion{
    v3 = ,
} TBSVersion;

typedef struct asn1TBSCertificate{
    TBSVersion version;
    int serialNumber;
    SignatureScheme signature;
    uchar issuer[50];
    asn1Validity validity;
    uchar subject[50];
    asn1SubjectPublicKeyInfo subjectPublicKeyInfo;
}asn1TBSCertificate;

typedef struct asn1Validity{
    //YYMMDDHHMMSSZ
    //Null terminated
    uchar notBefore[14];
    uchar notAfter[14];
}asn1Validity;

typedef struct asn1SubjectPublicKeyInfo{
    SignatureScheme algorithm;
    PublicKey subjectPublicKey;
}asn1SubjectPublicKeyInfo;


typedef struct DER{
    uchar *data;
    int lenData;
}DER;

typedef struct Base64{
    uchar *data;
    int lenData;
}Base64;

void generateX509(KeyPair kp){
    asn1Certificate ans1Certif = generateAsn1X509(kp);
    DER derCertif = asn1ToDER(asn1Certif);
    Base64 b64Certif = base64Encode(derCertif.data,derCertif.lenData);
    uchar pemTemplate[] = "-----BEGIN CERTIFICATE-----\n%s\n-----END CERTIFICATE-----\n";
    //Sub 1 for the format specifier and sub 1 for base64 null terminator
    FILE *fhand;
    fhand = fopen("certif.pem","w");
    fprintf(fhand,pemTemplate,b64Certif.data);
    fclose(fhand);

    free(derCertif.data);
    free(b64Certif.data);

}


asn1Certificate generateAsn1X509(KeyPair kp){
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
    certif.signatureValue = decryptRSA(hash,8,kp);
    free(hash);
    free(der.data);
}
#define DER_SEQUENCE 0x30
#define DER_INTEGER 0x02
#define DER_UTF8STRING 0x0C
#define DER_BITSTRING 0x03
DER asn1TBSToDER(asn1TBSCertificate asn1TBSCertif){
    DER result;
    result.data = (uchar*) malloc(1000);
    if(!result.data){
        allocError();
    }
    int index = 0;
    //TODO add sequence lengths
    result.data[index++] = DER_SEQUENCE;
    index += derEncodeInt(&result.data[index],asn1TBSCertif.version);
    index += derEncodeInt(&result.data[index],asn1TBSCertif.serialNumber);
    index += derEncodeInt(&result.data[index],asn1TBSCertif.signature);
    index += derEncodeString(&result.data[index],asn1TBSCertif.issuer,sizeof(asn1TBSCertif.issuer));
    result.data[index++] = DER_SEQUENCE;
    index += derEncodeString(&result.data[index],asn1TBSCertif.validity.notBefore,sizeof(asn1TBSCertif.validity.notBefore));
    index += derEncodeString(&result.data[index],asn1TBSCertif.validity.notAfter,sizeof(asn1TBSCertif.validity.notAfter));
    index += derEncodeString(&result.data[index],asn1TBSCertif.subject,sizeof(asn1TBSCertif.subject));
    result.data[index++] = DER_SEQUENCE;
    index += derEncodeInt(&result.data[index],asn1TBSCertif.subjectPublicKeyInfo.algorithm);
    result.data[index++] = DER_SEQUENCE;
    index += derEncodeInt(&result.data[index],asn1TBSCertif.subjectPublicKeyInfo.subjectPublicKey.e);
    index += derEncodeBignum(&result.data[index],asn1TBSCertif.subjectPublicKeyInfo.subjectPublicKey.n,asn1TBSCertif.subjectPublicKeyInfo.subjectPublicKey.lenN);

    uchar *resizedResult = (uchar*) malloc(index);
    free(result.data);
    result.data = resizedResult;
    result.lenData = index;
    return result;
}

int derEncodeBignum(uchar *result,bignum n,int lenN){
    result[0] = DER_BITSTRING;
    result[1] = lenN*4;
    for(int i=0;i<lenN*4;i++){
        //big endian
        result[i+2] = (n[i/4] >> (3-(i&0x3))) & 0xff;
    }
    return lenN*4+2;
}


int derEncodeString(uchar *result,uchar *string,int lenString){
    result[0] = DER_UTF8STRING;
    result[1] = lenString;
    for(int i=0;i<lenString;i++){
        result[i+2] = string[i];
    }
    return lenString+2;
}

int derEncodeInt(uchar *result,int num){
    result[0] = DER_INTEGER;
    result[1] = 4;
    for(int i=0;i<4;i++){
        //big endian
        result[2+i] = (num >> ((3-i)*8)) & 0xff;
    }
    return 6;
}

DER asn1ToDER(asn1Certificate asn1Certif){
    //Define a bunch of macros
    //E.g. SEQUENCE = 0x30
    //Hard-coded parse the structs with TLV
}

//1hr 36mins
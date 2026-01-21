#include "x509.h"
#include <stdio.h>
#include <string.h>
#include "sha.h"
#include "base64.h"

//An implementation of an X509 certificate creation roughly following RFC 5280
#define DER_SEQUENCE 0x30



static asn1Certificate x509ToAsn1(uchar *fname);
static asn1Certificate generateAsn1X509(RSAPublicKey subjectPk,RSAKeyPair issuerKp);
static String asn1TBSToDER(asn1TBSCertificate asn1TBSCertif);
static String asn1ToDER(asn1Certificate asn1Certif);
static void readX509(uchar *fname,uchar *buff,int lenBuff,int *startIndex,int *endIndex);
static asn1Certificate derToAsn1(String der);
static asn1TBSCertificate derTBSToAsn1(String der,int *index);




void generateX509(RSAPublicKey subjectPk,RSAKeyPair issuerKp,uchar *fname){
    asn1Certificate asn1Certif = generateAsn1X509(subjectPk,issuerKp);
    String derCertif = asn1ToDER(asn1Certif);
    String b64Certif = base64Encode(derCertif);
    uchar pemTemplate[] = "-----BEGIN CERTIFICATE-----\n%s\n-----END CERTIFICATE-----";
    FILE *fhand = fopen(fname,"w");
    fprintf(fhand,pemTemplate,b64Certif.data);
    fclose(fhand);

    free(derCertif.data);
    free(b64Certif.data);
}


static asn1Certificate generateAsn1X509(RSAPublicKey subjectPk,RSAKeyPair issuerKp){
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

    String der = asn1TBSToDER(certif.tbsCertif);
    bignum hash = sha256(der.data,der.lenData);
    //Not decrypting, just using the private key to encrypt
    uchar *signatureString = calloc(issuerKp.publicKey.lenN*sizeof(uint32_t),sizeof(uchar));
    decryptRSA(hash,8,issuerKp,signatureString,issuerKp.publicKey.lenN*sizeof(uint32_t));
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

static String asn1ToDER(asn1Certificate asn1Certif){
    String result;
    //Allocate a buffer that is sufficient in size
    //We will resize when we know the length
    result.data = (uchar*) malloc(2048);
    if(!result.data){
        allocError();
    }
    int index = 0;

    //Certif sequence {
    result.data[index++] = DER_SEQUENCE;
    int certifSequenceLengthIndex = index;
    //reserve the index
    index += derEncodeInt(&result.data[index],0);
    int certifSequenceStart = index;
    
    //  tbsCertif {
    String tbsDER = asn1TBSToDER(asn1Certif.tbsCertif);
    memcpy(&result.data[index],tbsDER.data,tbsDER.lenData);
    index += tbsDER.lenData;
    free(tbsDER.data);
    //  } tbsCertif 

    //  Signature Algorithm {
    index += derEncodeInt(&result.data[index],asn1Certif.signatureAlgorithm);
    //  }


    //  Signature value and length {
    index += derEncodeBignum(&result.data[index],asn1Certif.signatureValue,asn1Certif.lenSignatureValue*sizeof(uint32_t));
    //  }

    derEncodeInt(&result.data[certifSequenceLengthIndex],index-certifSequenceStart);
    //} Certif sequence

    uchar *resizedResult = realloc(result.data,index);
    if(!resizedResult){
        allocError();
    }
    result.data = resizedResult;
    result.lenData = index;
    return result;
}

static String asn1TBSToDER(asn1TBSCertificate asn1TBSCertif){
    String result;
    //Allocate a buffer that is sufficient in size
    //We will resize when we know the length
    result.data = (uchar*) malloc(2048);
    if(!result.data){
        allocError();
    }
    int index = 0;
    
    //TBS certif sequence {
    result.data[index++] = DER_SEQUENCE;
    int tbsSequenceLengthIndex = index;
    //reserve the index
    index += derEncodeInt(&result.data[index],0);
    int tbsSequenceStart = index;
    index += derEncodeInt(&result.data[index],asn1TBSCertif.version);
    index += derEncodeInt(&result.data[index],asn1TBSCertif.serialNumber);
    index += derEncodeInt(&result.data[index],asn1TBSCertif.signature);
    index += derEncodeString(&result.data[index],asn1TBSCertif.issuer,sizeof(asn1TBSCertif.issuer));
    

    //  Vaildity {
    result.data[index++] = DER_SEQUENCE;
    int validitySequenceLengthIndex = index;
    //reserve index
    index += derEncodeInt(&result.data[index],0);
    int validitySequenceStart = index;
    index += derEncodeString(&result.data[index],asn1TBSCertif.validity.notBefore,sizeof(asn1TBSCertif.validity.notBefore));
    index += derEncodeString(&result.data[index],asn1TBSCertif.validity.notAfter,sizeof(asn1TBSCertif.validity.notAfter));

    derEncodeInt(&result.data[validitySequenceLengthIndex],index-validitySequenceStart);
    //  } Validity

    //  Subject {
    index += derEncodeString(&result.data[index],asn1TBSCertif.subject,sizeof(asn1TBSCertif.subject));    
    //  } Subject

    //  Public key info {
    result.data[index++] = DER_SEQUENCE;
    int keyInfoSequenceLengthIndex = index;
    index += derEncodeInt(&result.data[index],0);
    int keyInfoSequenceStart = index;
    index += derEncodeInt(&result.data[index],asn1TBSCertif.subjectPublicKeyInfo.algorithm);

    //      RSAPublicKey {
    result.data[index++] = DER_SEQUENCE;
    int rsaPublicKeySequenceLengthIndex = index;
    index += derEncodeInt(&result.data[index],0);
    int rsaPublicKeySequenceStart = index;
    index += derEncodeInt(&result.data[index],asn1TBSCertif.subjectPublicKeyInfo.subjectPublicKey.e);
    index += derEncodeBignum(&result.data[index],asn1TBSCertif.subjectPublicKeyInfo.subjectPublicKey.n,asn1TBSCertif.subjectPublicKeyInfo.subjectPublicKey.lenN);
    
    derEncodeInt(&result.data[keyInfoSequenceLengthIndex],index-keyInfoSequenceStart);
    //      } RSAPublicKey
    derEncodeInt(&result.data[rsaPublicKeySequenceLengthIndex],index-rsaPublicKeySequenceStart);
    //  } Public key info
    derEncodeInt(&result.data[tbsSequenceLengthIndex],index-tbsSequenceStart);
    //} TBS certif sequence

    uchar *resizedResult = realloc(result.data,index);
    if(!resizedResult){
        allocError();
    }
    result.data = resizedResult;
    result.lenData = index;
    return result;
}




certifStatus checkX509(RSAPublicKey issuerPk,uchar *fname){
    asn1Certificate asn1Cerif =  x509ToAsn1(fname);
    certifStatus result;
    result = VALID;
    return result;
    //TODO
    //Check signature 
    //Check date
}

static asn1Certificate x509ToAsn1(uchar *fname){
    const int lenBuff = 2048;
    uchar buff[2048] = {0};
    int startIndex,endIndex;
    readX509(fname,buff,lenBuff,&startIndex,&endIndex);
    String rawX509 = {&buff[startIndex],endIndex};
    String derCertif = base64Decode(rawX509);
    asn1Certificate asn1Certif = derToAsn1(derCertif);
    free(derCertif.data);
    return asn1Certif;
}

static void readX509(uchar *fname,uchar *buff,int lenBuff,int *startIndex,int *endIndex){
    FILE *fhand = fopen(fname,"r");
    
    int currFileLength = 0;
    while(fgets(&buff[currFileLength],lenBuff-currFileLength,fhand)){
        for(;currFileLength<lenBuff && buff[currFileLength];currFileLength++);
    }
    fclose(fhand);
    
    uchar beginCertif[] = "-----BEGIN CERTIFICATE-----\n";
    uchar endCertif[] = "\n-----END CERTIFICATE-----";

    int i=0;
    //-1 to account for null terminator
    for(;i<sizeof(beginCertif)-1;i++){
        if(buff[i]!=beginCertif[i]){
            perror("x509ToAsn1: Certificate does not have the correct header");
            exit(1);
        }
    }
    int j = 0;
    // i == sizeof(beginCertif)-1
    for(;i<lenBuff;i++){
        if(j>=1){
            if(buff[i]!=endCertif[j]){
                perror("x509ToAsn1: Certificate does not have the correct header");
                exit(1);
            }
            j++;
            //Null terminator
            if(j==sizeof(endCertif)-1){
                i++;
                break;
            }
        }
        else if(buff[i]==endCertif[j]){
            j++;
        }
    }
    *startIndex = sizeof(beginCertif)-1;
    *endIndex = i-(sizeof(beginCertif)-1)-(sizeof(endCertif)-1);
}





static asn1Certificate derToAsn1(String der){
    asn1Certificate result;
    int index = 0;
    if(der.data[index++]!=DER_SEQUENCE){
        perror("derToAsn1: Malformed DER certificate");
        exit(1);
    }
    int derCertifLength = derDecodeInt(der.data,&index);
    if(derCertifLength!=der.lenData-index){ //Minus the start seq and length
        perror("derToAsn1: Malformed DER certificate. Length is incorrect.");
        exit(1);
    }
    typedef enum Asn1Field{
        TBS,
        SignatureAlg,
        SignatureVal
    } Asn1Field;
    Asn1Field currField = TBS;
    while(index<der.lenData){
        switch(currField){
            case TBS:
                result.tbsCertif = derTBSToAsn1(der,&index);
                break;
            case SignatureAlg:
                result.signatureAlgorithm = derDecodeInt(der.data,&index);
                break;
            case SignatureVal:
                result.signatureValue = derDecodeBignum(der.data,&index,&result.lenSignatureValue);
                break;
            default:
                perror("derToAsn1: Malformed DER certificate. Unknown field.");
                exit(1);
        };
        currField++;
    }
    return result;
}




static asn1TBSCertificate derTBSToAsn1(String der,int *index){
    asn1TBSCertificate result;
    if(der.data[(*index)++]!=DER_SEQUENCE){
        perror("derTBSToAsn1: Malformed DER TBS certificate");
        exit(1);
    }
    int lenTBSSequence = derDecodeInt(der.data,index);
    typedef enum tbsAsn1Field{
        Version,
        SerialNumber,
        Signature,
        Issuer,
        Validity,
        Subject,
        SubjectPublicKeyInfo
    } tbsAsn1Field;

    int endIndex = *index+lenTBSSequence;
    tbsAsn1Field currField = Version;
    while(*index<endIndex){
        switch(currField){
            case Version:
                result.version = derDecodeInt(der.data,index);
                break;
            case SerialNumber:
                result.serialNumber = derDecodeInt(der.data,index);
                break;
            case Signature:
                result.signature = derDecodeInt(der.data,index);
                break;
            case Issuer:
                int lenIssuer = 0;
                uchar *issuer = derDecodeString(der.data,index,&lenIssuer);
                int lenIssuerField = sizeof(result.issuer)/sizeof(result.issuer[0]);
                if(lenIssuer>lenIssuerField){
                    perror("derTBSToAsn1: Issuer string length is too long");
                    exit(1);
                }
                memset(result.issuer,0,lenIssuerField);
                memcpy(result.issuer,issuer,lenIssuer);
                break;
            case Validity:
                if(der.data[(*index)++]!=DER_SEQUENCE){
                    perror("derTBSToAsn1: Malformed DER TBS certificate. Unknown field.");
                    exit(1);
                }
                int length = derDecodeInt(der.data,index);
                int startIndex = *index;
                int lenNotBefore = 0;
                uchar *notBefore = derDecodeString(der.data,index,&lenNotBefore);
                int lenNotBeforeField = sizeof(result.validity.notBefore)/sizeof(result.validity.notBefore[0]);
                if(lenNotBefore>lenNotBeforeField){
                    perror("derTBSToAsn1: notBefore string length is too long");
                    exit(1);
                }
                memset(result.validity.notBefore,0,lenNotBeforeField);
                memcpy(result.validity.notBefore,notBefore,lenNotBefore);

                int lenNotAfter = 0;
                uchar *notAfter = derDecodeString(der.data,index,&lenNotAfter);
                int lenNotAfterField = sizeof(result.validity.notAfter)/sizeof(result.validity.notAfter[0]);
                if(lenNotAfter>lenNotAfterField){
                    perror("derTBSToAsn1: notAfter string length is too long");
                    exit(1);
                }
                memset(result.validity.notAfter,0,lenNotAfterField);
                memcpy(result.validity.notAfter,notAfter,lenNotAfter);
                if(startIndex+length!=*index){
                    perror("derTBSToAsn1: Malformed DER TBS certificate. Length mismatch.");
                    exit(1);
                }
                break;
            case Subject:
                int lenSubject = 0;
                uchar *subject = derDecodeString(der.data,index,&lenSubject);
                int lenSubjectField = sizeof(result.subject)/sizeof(result.subject[0]);
                if(lenSubject>lenSubjectField){
                    perror("derTBSToAsn1: Subject string length is too long");
                    exit(1);
                }
                memset(result.subject,0,lenSubjectField);
                memcpy(result.subject,subject,lenSubject);
                break;
            case SubjectPublicKeyInfo:
                if(der.data[(*index)++]!=DER_SEQUENCE){
                    perror("derTBSToAsn1: Malformed DER TBS certificate. Unknown field.");
                    exit(1);
                }
                int fieldLength = derDecodeInt(der.data,index);
                int startFieldIndex = *index;
                result.subjectPublicKeyInfo.algorithm = derDecodeInt(der.data,index);
                if(der.data[(*index)++]!=DER_SEQUENCE){
                    perror("derTBSToAsn1: Malformed DER TBS certificate. Unknown field.");
                    exit(1);
                }
                int subFieldLength = derDecodeInt(der.data,index);
                int startSubFieldIndex = *index;
                result.subjectPublicKeyInfo.subjectPublicKey.e = derDecodeInt(der.data,index);
                result.subjectPublicKeyInfo.subjectPublicKey.n = derDecodeBignum(der.data,index,&result.subjectPublicKeyInfo.subjectPublicKey.lenN);
                if(startSubFieldIndex+subFieldLength!=*index || startFieldIndex+fieldLength!=*index){
                    perror("derTBSToAsn1: Malformed DER TBS certificate. Length mismatch.");
                    exit(1);
                }
                break;
            default:
                perror("derTBSToAsn1: Malformed DER TBS certificate. Unknown field.");
                exit(1);
        };
        currField++;
    }
    return result;
}


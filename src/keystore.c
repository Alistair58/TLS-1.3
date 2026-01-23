#include "keystore.h"
#include "base64.h"
#include "der.h"

#define DER_SEQUENCE 0x30

static String privateKeyToDER(RSAPrivateKey pk);
static String publicKeyToDER(RSAPublicKey pk);
static RSAPublicKey derToPublicKey(String inp);
static RSAPrivateKey derToPrivateKey(String inp);


void savePublicKey(RSAPublicKey pk,uchar *fname){
    //If we wanted to do it officially, we could convert to ASN1 but it's just going to be the same as the RSAPublicKey struct
    String der = publicKeyToDER(pk);
    String b64 = base64Encode(der);
    uchar pemTemplate[] = "-----BEGIN PUBLIC KEY-----\n%s\n-----END PUBLIC KEY-----";
    FILE *fhand = fopen(fname,"w");
    fprintf(fhand,pemTemplate,b64.data);
    fclose(fhand);

    free(der.data);
    free(b64.data);
}
void savePrivateKey(RSAPrivateKey pk,uchar *fname){
    String der = privateKeyToDER(pk);
    String b64 = base64Encode(der);
    uchar pemTemplate[] = "-----BEGIN PRIVATE KEY-----\n%s\n-----END PRIVATE KEY-----";
    FILE *fhand = fopen(fname,"w");
    fprintf(fhand,pemTemplate,b64.data);
    fclose(fhand);

    free(der.data);
    free(b64.data);
}

RSAPublicKey readPublicKey(uchar *fname){
    String buff;
    buff.lenData = 2048;
    buff.data = (uchar*) malloc(buff.lenData);
    
    FILE *fhand = fopen(fname,"r");
    int currFileLength = 0;
    while(fgets(&buff.data[currFileLength],buff.lenData-currFileLength,fhand)){
        for(;currFileLength<buff.lenData && buff.data[currFileLength];currFileLength++);
    }
    fclose(fhand);

    uchar beginKey[] = "-----BEGIN PUBLIC KEY-----\n";
    uchar endKey[] = "\n-----END PUBLIC KEY-----";

    int i=0;
    //-1 to account for null terminator
    for(;i<sizeof(beginKey)-1;i++){
        if(buff.data[i]!=beginKey[i]){
            perror("readPublicKey: Key store does not have the correct header");
            exit(1);
        }
    }
    int j = 0;
    // i == sizeof(beginKey)-1
    for(;i<buff.lenData;i++){
        if(j>=1){
            if(buff.data[i]!=endKey[j]){
                perror("readPublicKey: Key store does not have the correct header");
                exit(1);
            }
            j++;
            //Null terminator
            if(j==sizeof(endKey)-1){
                i++;
                break;
            }
        }
        else if(buff.data[i]==endKey[j]){
            j++;
        }
    }
    int startBodyIndex = sizeof(beginKey)-1;
    int endBodyIndex = i-(sizeof(beginKey)-1)-(sizeof(endKey)-1);

    
    String pemBody = {&buff.data[startBodyIndex],endBodyIndex};
    String der = base64Decode(pemBody);
    free(buff.data);

    RSAPublicKey result = derToPublicKey(der);
    free(der.data);    
    return result;
}

RSAPrivateKey readPrivateKey(uchar *fname){
    String buff;
    buff.lenData = 2048;
    buff.data = (uchar*) malloc(buff.lenData);
    
    FILE *fhand = fopen(fname,"r");
    int currFileLength = 0;
    while(fgets(&buff.data[currFileLength],buff.lenData-currFileLength,fhand)){
        for(;currFileLength<buff.lenData && buff.data[currFileLength];currFileLength++);
    }
    fclose(fhand);

    uchar beginKey[] = "-----BEGIN PRIVATE KEY-----\n";
    uchar endKey[] = "\n-----END PRIVATE KEY-----";

    int i=0;
    //-1 to account for null terminator
    for(;i<sizeof(beginKey)-1;i++){
        if(buff.data[i]!=beginKey[i]){
            perror("readPrivateKey: Key store does not have the correct header");
            exit(1);
        }
    }
    int j = 0;
    // i == sizeof(beginKey)-1
    for(;i<buff.lenData;i++){
        if(j>=1){
            if(buff.data[i]!=endKey[j]){
                perror("readPrivateKey: Key store does not have the correct header");
                exit(1);
            }
            j++;
            //Null terminator
            if(j==sizeof(endKey)-1){
                i++;
                break;
            }
        }
        else if(buff.data[i]==endKey[j]){
            j++;
        }
    }
    int startBodyIndex = sizeof(beginKey)-1;
    int endBodyIndex = i-(sizeof(beginKey)-1)-(sizeof(endKey)-1);

    
    String pemBody = {&buff.data[startBodyIndex],endBodyIndex};
    String der = base64Decode(pemBody);
    free(buff.data);

    RSAPrivateKey result = derToPrivateKey(der);
    free(der.data);    
    return result;
}


static String privateKeyToDER(RSAPrivateKey pk){
    String result;
    result.data = (uchar*) malloc(2048);
    if(!result.data){
        allocError();
    }
    int index = 0;

    result.data[index++] = DER_SEQUENCE;
    //Reserve the index for the length
    int privateKeyLengthIdx = index;
    index += derEncodeInt(&result.data[index],0);
    int privateKeyStart = index;

    index += derEncodeBignum(&result.data[index],pk.p,pk.lenP);
    index += derEncodeBignum(&result.data[index],pk.q,pk.lenQ);

    //Put the correct length in
    derEncodeInt(&result.data[privateKeyLengthIdx],index-privateKeyStart);

    uchar *resized = realloc(result.data,index);
    if(!resized){
        allocError();
    }
    result.data = resized;
    result.lenData = index;
    return result;
}


static String publicKeyToDER(RSAPublicKey pk){
    String result;
    result.data = (uchar*) malloc(2048);
    if(!result.data){
        allocError();
    }
    int index = 0;

    result.data[index++] = DER_SEQUENCE;
    //Reserve the index for the length
    int publicKeyLengthIdx = index;
    index += derEncodeInt(&result.data[index],0);
    int publicKeyStart = index;

    index += derEncodeBignum(&result.data[index],pk.n,pk.lenN);
    index += derEncodeInt(&result.data[index],pk.e);

    //Put the correct length in
    derEncodeInt(&result.data[publicKeyLengthIdx],index-publicKeyStart);

    uchar *resized = realloc(result.data,index);
    if(!resized){
        allocError();
    }
    result.data = resized;
    result.lenData = index;
    return result;
}

static RSAPrivateKey derToPrivateKey(String der){
    RSAPrivateKey result;
    int index = 0;

    if(der.data[index++]!=DER_SEQUENCE){
        perror("derToPrivateKey: Malformed DER key store");
        exit(1);
    }
    int declaredLength = derDecodeInt(der.data,&index);
    int startPrivateKeyIdx = index;

    result.p = derDecodeBignum(der.data,&index,&result.lenP);
    result.q = derDecodeBignum(der.data,&index,&result.lenQ);

    if(index-startPrivateKeyIdx != declaredLength){
        perror("derToPrivateKey: Malformed DER key store. Length is incorrect.");
        exit(1);
    }

    return result;
}

static RSAPublicKey derToPublicKey(String der){
    RSAPublicKey result;
    int index = 0;

    if(der.data[index++]!=DER_SEQUENCE){
        perror("derToPublicKey: Malformed DER key store");
        exit(1);
    }
    int declaredLength = derDecodeInt(der.data,&index);
    int startPublicKeyIdx = index;

    result.n = derDecodeBignum(der.data,&index,&result.lenN);
    result.e = derDecodeInt(der.data,&index);

    if(index-startPublicKeyIdx != declaredLength){
        perror("derToPublicKey: Malformed DER key store. Length is incorrect.");
        exit(1);
    }

    return result;
}


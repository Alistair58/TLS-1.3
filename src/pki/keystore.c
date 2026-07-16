#include "../pki/keystore.h"
#include "../pki/base64.h"
#include "../pki/der.h"
#include <string.h>

static String publicKeyToDER(RSAPublicKey pk);
static String privateKeyToDER(RSAPrivateKey pk);
static String keyPairToDER(RSAKeyPair kp);
static RSAPublicKey derToPublicKey(String inp,int *index);
static RSAPrivateKey derToPrivateKey(String inp,int *index);
static RSAKeyPair derToKeyPair(String inp,int *index);


void savePublicKey(RSAPublicKey pk,char *fname){
    //If we wanted to do it officially, we could convert to ASN1 but it's just going to be the same as the RSAPublicKey struct
    String der = publicKeyToDER(pk);
    String b64 = base64Encode(der,true);
    uchar pemTemplate[] = "-----BEGIN PUBLIC KEY-----\n%s\n-----END PUBLIC KEY-----";
    FILE *fhand = fopen(fname,"w");
    if(!fhand){
        fprintf(stderr,"savePublicKey: Could not locate or create %s\n",fname);
        exit(1);
    }
    fprintf(fhand,pemTemplate,b64.data);
    fclose(fhand);

    free(der.data);
    free(b64.data);
}
void saveKeyPair(RSAKeyPair kp,char *fname){
    String der = keyPairToDER(kp);
    String b64 = base64Encode(der,true);
    uchar pemTemplate[] = "-----BEGIN KEY PAIR-----\n%s\n-----END KEY PAIR-----";
    FILE *fhand = fopen(fname,"w");
    if(!fhand){
        fprintf(stderr,"saveKeyPair: Could not locate or create %s\n",fname);
        exit(1);
    }
    fprintf(fhand,pemTemplate,b64.data);
    fclose(fhand);

    free(der.data);
    free(b64.data);
}

RSAPublicKey readPublicKey(char *fname){
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
    String der = base64Decode(pemBody,false);
    free(buff.data);

    RSAPublicKey result = derToPublicKey(der,NULL);
    free(der.data);    
    return result;
}

RSAKeyPair readKeyPair(char *fname){
    String buff;
    buff.lenData = 2048;
    buff.data = (uchar*) malloc(buff.lenData);
    
    FILE *fhand = fopen(fname,"r");
    int currFileLength = 0;
    while(fgets(&buff.data[currFileLength],buff.lenData-currFileLength,fhand)){
        for(;currFileLength<buff.lenData && buff.data[currFileLength];currFileLength++);
    }
    fclose(fhand);

    uchar beginKey[] = "-----BEGIN KEY PAIR-----\n";
    uchar endKey[] = "\n-----END KEY PAIR-----";

    int i=0;
    //-1 to account for null terminator
    for(;i<sizeof(beginKey)-1;i++){
        if(buff.data[i]!=beginKey[i]){
            perror("readKeyPair: Key store does not have the correct header");
            exit(1);
        }
    }
    int j = 0;
    // i == sizeof(beginKey)-1
    for(;i<buff.lenData;i++){
        if(j>=1){
            if(buff.data[i]!=endKey[j]){
                perror("readKeyPair: Key store does not have the correct header");
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
    String der = base64Decode(pemBody,false);
    free(buff.data);

    RSAKeyPair result = derToKeyPair(der,NULL);
    free(der.data);    
    return result;
}

static String keyPairToDER(RSAKeyPair kp){
    String result;
    result.lenData = 2048;
    result.data = (uchar*) malloc(result.lenData);
    if(!result.data){
        allocError();
    }
    int index = 0;

    result.data[index++] = DER_SEQUENCE;
    //Reserve the index for the length
    int keyPairLengthIdx = index;
    index += derEncodeInt(&result.data[index],result.lenData-index,0);
    int keyPairStart = index;

    String publicKeyDer = publicKeyToDER(kp.publicKey);
    memcpy(&result.data[index],publicKeyDer.data,publicKeyDer.lenData);
    index += publicKeyDer.lenData;
    String privateKeyDer = privateKeyToDER(kp.privateKey);
    memcpy(&result.data[index],privateKeyDer.data,privateKeyDer.lenData);
    index += privateKeyDer.lenData;

    //Put the correct length in
    derEncodeInt(&result.data[keyPairLengthIdx],result.lenData-keyPairLengthIdx,index-keyPairStart);

    uchar *resized = realloc(result.data,index);
    if(!resized){
        allocError();
    }
    result.data = resized;
    result.lenData = index;
    free(publicKeyDer.data);
    free(privateKeyDer.data);
    return result;
}


static String privateKeyToDER(RSAPrivateKey pk){
    String result;
    result.lenData = 2048;
    result.data = (uchar*) malloc(result.lenData);
    if(!result.data){
        allocError();
    }
    int index = 0;

    result.data[index++] = DER_SEQUENCE;
    //Reserve the index for the length
    int privateKeyLengthIdx = index;
    index += derEncodeInt(&result.data[index],result.lenData-index,0);
    int privateKeyStart = index;

    index += derEncodeBignum(&result.data[index],result.lenData-index,pk.p,pk.lenP);
    index += derEncodeBignum(&result.data[index],result.lenData-index,pk.q,pk.lenQ);

    //Put the correct length in
    derEncodeInt(&result.data[privateKeyLengthIdx],result.lenData-privateKeyLengthIdx,index-privateKeyStart);

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
    result.lenData = 2048;
    result.data = (uchar*) malloc(result.lenData);
    if(!result.data){
        allocError();
    }
    int index = 0;

    result.data[index++] = DER_SEQUENCE;
    //Reserve the index for the length
    int publicKeyLengthIdx = index;
    index += derEncodeInt(&result.data[index],result.lenData-index,0);
    int publicKeyStart = index;

    index += derEncodeBignum(&result.data[index],result.lenData-index,pk.n,pk.lenN);
    index += derEncodeInt(&result.data[index],result.lenData-index,pk.e);

    //Put the correct length in
    derEncodeInt(&result.data[publicKeyLengthIdx],result.lenData-publicKeyLengthIdx,index-publicKeyStart);

    uchar *resized = realloc(result.data,index);
    if(!resized){
        allocError();
    }
    result.data = resized;
    result.lenData = index;
    return result;
}

static RSAKeyPair derToKeyPair(String der,int *index){
    int zero = 0;
    if(NULL == index){
        index = &zero;
    }
    RSAKeyPair result;

    if(der.data[(*index)++]!=DER_SEQUENCE){
        perror("derToKeyPair: Malformed DER key store");
        exit(1);
    }
    int declaredLength = derDecodeInt(der.data,der.lenData,index);
    int startPrivateKeyIdx = *index;

    result.publicKey = derToPublicKey(der,index);
    result.privateKey = derToPrivateKey(der,index);

    if(*index-startPrivateKeyIdx != declaredLength){
        perror("derToKeyPair: Malformed DER key store. Length is incorrect.");
        exit(1);
    }

    return result;
}


static RSAPrivateKey derToPrivateKey(String der,int *index){
    int zero = 0;
    if(NULL == index){
        index = &zero;
    }

    RSAPrivateKey result;

    if(der.data[(*index)++]!=DER_SEQUENCE){
        perror("derToPrivateKey: Malformed DER key store");
        exit(1);
    }
    int declaredLength = derDecodeInt(der.data,der.lenData,index);
    int startPrivateKeyIdx = *index;

    result.p = derDecodeBignum(der.data,der.lenData,index,&result.lenP);
    result.q = derDecodeBignum(der.data,der.lenData,index,&result.lenQ);

    if(*index-startPrivateKeyIdx != declaredLength){
        perror("derToPrivateKey: Malformed DER key store. Length is incorrect.");
        exit(1);
    }

    return result;
}

static RSAPublicKey derToPublicKey(String der,int *index){
    int zero = 0;
    if(NULL == index){
        index = &zero;
    }
    RSAPublicKey result;

    if(der.data[(*index)++]!=DER_SEQUENCE){
        perror("derToPublicKey: Malformed DER key store");
        exit(1);
    }
    int declaredLength = derDecodeInt(der.data,der.lenData,index);
    int startPublicKeyIdx = *index;

    result.n = derDecodeBignum(der.data,der.lenData,index,&result.lenN);
    result.e = derDecodeInt(der.data,der.lenData,index);

    if(*index-startPublicKeyIdx != declaredLength){
        perror("derToPublicKey: Malformed DER key store. Length is incorrect.");
        exit(1);
    }

    return result;
}


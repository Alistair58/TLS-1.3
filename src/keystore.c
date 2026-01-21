#include "keystore.h"
#include "base64.h"

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
    
    FILE *fhand = fopen(fname,"w");
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
    String der = base64Decode(buff);
    free(buff.data);

    RSAPublicKey result = derToPublicKey(der);
    free(der.data);    
    return result;
}

RSAPrivateKey readPrivateKey(uchar *fname){

}


static String privateKeyToDER(RSAPrivateKey pk){

}


static String publicKeyToDER(RSAPublicKey pk){

}

static RSAPublicKey derToPublicKey(String der){

}

static RSAPrivateKey derToPrivateKey(String der){

}
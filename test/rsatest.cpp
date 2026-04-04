extern "C" {
#include "../src/rsa.h"
}
#include <gtest/gtest.h>


TEST(RSATest,rsaEncryptionDecryption){
    RSAKeyPair kp = generateKeys(128);
    uchar msg[] = "abc";
    bignum dest = (bignum) calloc(kp.publicKey.lenN,sizeof(uint32_t));
    if(!dest){
        allocError();
    }
    encryptRSA(msg,sizeof(msg),kp.publicKey,dest,kp.publicKey.lenN);
    uchar result[sizeof(msg)];
    decryptRSA(dest,kp.publicKey.lenN,kp,result,sizeof(msg));
    for(int i=0;i<sizeof(msg);i++){
        EXPECT_EQ(result[i],msg[i]);
    }
    freeRSAKeyPair(kp);
    free(dest);
}


TEST(RSATest,rsaDecryptionEncryption){
    RSAKeyPair kp = generateKeys(128);
    uchar msg[] = "abc";
    uchar *dest = (uchar*) calloc(kp.publicKey.lenN*sizeof(uint32_t),sizeof(uchar));
    if(!dest){
        freeRSAKeyPair(kp);
        allocError();
    }
    decryptRSA((bignum)msg,sizeof(msg)/sizeof(uint32_t),kp,dest,kp.publicKey.lenN*sizeof(uint32_t));
    uchar *result = (uchar*) calloc(kp.publicKey.lenN*sizeof(uint32_t),sizeof(uchar));
    if(!result){
        freeRSAKeyPair(kp);
        free(dest);
        allocError();
    }
    encryptRSA(dest,kp.publicKey.lenN*sizeof(uint32_t),kp.publicKey,(bignum)result,kp.publicKey.lenN);
    for(int i=0;i<sizeof(msg);i++){
        EXPECT_EQ(result[kp.publicKey.lenN*sizeof(uint32_t)-sizeof(msg)+i],msg[i]);
    }
    freeRSAKeyPair(kp);
    free(dest);
    free(result);
}
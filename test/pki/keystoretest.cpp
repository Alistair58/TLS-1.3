extern "C" {
#include "../../src/pki/keystore.h"
#include "../../src/crypto/rsa.h"

}
#include <gtest/gtest.h>



TEST(KeystoreTest,saveAndReadPublicKey){
    RSAKeyPair kp = generateKeys(64);
    char *fname = "./test/tmp/test.pem";
    savePublicKey(kp.publicKey,fname);
    RSAPublicKey res = readPublicKey(fname);
    ASSERT_EQ(res.e,kp.publicKey.e);
    ASSERT_EQ(res.lenN,kp.publicKey.lenN);
    for(int i=0;i<res.lenN;i++){
        ASSERT_EQ(res.n[i],kp.publicKey.n[i]);
    }
    freeRSAKeyPair(kp);
    freeRSAPublicKey(res);
}

TEST(KeystoreTest,saveAndReadKeyPair){
    RSAKeyPair kp = generateKeys(64);
    char *fname = "./test/tmp/test.pem";
    saveKeyPair(kp,fname);
    RSAKeyPair res = readKeyPair(fname);
    ASSERT_EQ(res.publicKey.e,kp.publicKey.e);
    ASSERT_EQ(res.publicKey.lenN,kp.publicKey.lenN);
    ASSERT_EQ(res.privateKey.lenP,kp.privateKey.lenP);
    ASSERT_EQ(res.privateKey.lenQ,kp.privateKey.lenQ);
    for(int i=0;i<res.publicKey.lenN;i++){
        ASSERT_EQ(res.publicKey.n[i],kp.publicKey.n[i]);
    }
    for(int i=0;i<res.privateKey.lenP;i++){
        ASSERT_EQ(res.privateKey.p[i],kp.privateKey.p[i]);
    }
    for(int i=0;i<res.privateKey.lenQ;i++){
        ASSERT_EQ(res.privateKey.q[i],kp.privateKey.q[i]);
    }
    freeRSAKeyPair(kp);
    freeRSAKeyPair(res);
}

TEST(KeystoreTest,readInvalidPublicKey){
    char *fname = "./test/tmp/test.pem";
    FILE *fhand = fopen(fname,"w");
    fprintf(fhand,"rubbish");
    fclose(fhand);

    EXPECT_EXIT(readPublicKey(fname),testing::ExitedWithCode(1),"readPublicKey: Key store does not have the correct header");
}

TEST(KeystoreTest,readInvalidKeyPair){
    char *fname = "./test/tmp/test.pem";
    FILE *fhand = fopen(fname,"w");
    fprintf(fhand,"rubbish");
    fclose(fhand);

    EXPECT_EXIT(readKeyPair(fname),testing::ExitedWithCode(1),"readKeyPair: Key store does not have the correct header");
}
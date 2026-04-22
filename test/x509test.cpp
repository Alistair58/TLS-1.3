extern "C" {
#include "../src/x509.h"
}
#include <gtest/gtest.h>


TEST(X509Test,checkX509Valid){
    //Need a key >256 bits so that it can encrypt a SHA 256 hash
    RSAKeyPair myKp = generateKeys(512);
    RSAKeyPair issuerKp = generateKeys(512);
    uchar fname[] = "./test/tmp/test.pem"; 
    generateX509(myKp.publicKey,issuerKp,fname);
    certifStatus status = checkX509(issuerKp.publicKey,fname);
    freeRSAKeyPair(myKp);
    freeRSAKeyPair(issuerKp);
    EXPECT_EQ(status,VALID);
}

TEST(X509Test,checkX509Make2){
    //Need a key >256 bits so that it can encrypt a SHA 256 hash
    RSAKeyPair myKp = generateKeys(512);
    RSAKeyPair issuerKp = generateKeys(512);
    uchar fname[] = "./test/tmp/test.pem"; 
    generateX509(myKp.publicKey,issuerKp,fname);
    certifStatus status1 = checkX509(issuerKp.publicKey,fname);
    generateX509(myKp.publicKey,issuerKp,fname);
    certifStatus status2 = checkX509(issuerKp.publicKey,fname);
    freeRSAKeyPair(myKp);
    freeRSAKeyPair(issuerKp);
    EXPECT_EQ(status1,VALID);
    EXPECT_EQ(status2,VALID);
}


TEST(X509Test,checkX509Empty){
    RSAKeyPair issuerKp = generateKeys(512);
    uchar fname[] = "./test/tmp/empty.pem"; 
    EXPECT_EXIT(checkX509(issuerKp.publicKey,fname),testing::ExitedWithCode(1),"x509ToAsn1: Certificate does not have the correct header");
    freeRSAKeyPair(issuerKp);

}

TEST(X509Test,checkX509Rubbish){
    RSAKeyPair issuerKp = generateKeys(512);
    uchar fname[] = "./test/tmp/rubbish.pem"; 
    EXPECT_EXIT(checkX509(issuerKp.publicKey,fname),testing::ExitedWithCode(1),"x509ToAsn1: Certificate does not have the correct header");
    freeRSAKeyPair(issuerKp);

}

TEST(X509Test,checkX509WrongSig){
    //Need a key >256 bits so that it can encrypt a SHA 256 hash
    RSAKeyPair myKp = generateKeys(512);
    RSAKeyPair issuerKp = generateKeys(512);
    issuerKp.publicKey.n[issuerKp.publicKey.lenN-1] ^= 1;
    uchar fname[] = "./test/tmp/test.pem"; 
    generateX509(myKp.publicKey,issuerKp,fname);
    certifStatus status = checkX509(issuerKp.publicKey,fname);
    freeRSAKeyPair(myKp);
    freeRSAKeyPair(issuerKp);
    EXPECT_EQ(status,INVALID_SIGNATURE);
}

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
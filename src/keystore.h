#ifndef KEYSTORE_H
#define KEYSTORE_H

#include "rsa.h"

//Parses to and from PEM format
void savePublicKey(RSAPublicKey pk,uchar *fname);
void savePrivateKey(RSAPrivateKey pk,uchar *fname);
RSAPublicKey readPublicKey(uchar *fname);
RSAPrivateKey readPrivateKey(uchar *fname);
#endif 
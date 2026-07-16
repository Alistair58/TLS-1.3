#ifndef KEYSTORE_H
#define KEYSTORE_H

#include "../crypto/rsa.h"

//Parses to and from PEM format
void savePublicKey(RSAPublicKey pk,char *fname);
void saveKeyPair(RSAKeyPair kp,char *fname);
RSAPublicKey readPublicKey(char *fname);
RSAKeyPair readKeyPair(char *fname);
#endif 
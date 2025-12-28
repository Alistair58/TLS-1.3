#ifndef SHA_H
#define SHA_H


#include <stdint.h>
#include "globals.h"
#include "bigmaths.h"

//lenMsg must be less than 2^61 bytes long - i.e. less than 2*10^6 terabytes
bignum sha256(uchar *msg,uint64_t lenMsg);


#endif
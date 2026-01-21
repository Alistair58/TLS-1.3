#ifndef DER_H
#define DER_H

#include "globals.h"
#include "bigmaths.h"

int derEncodeBignum(uchar *result,bignum n,int lenN);
int derEncodeString(uchar *result,uchar *string,int lenString);
int derEncodeInt(uchar *result,int num);

bignum derDecodeBignum(uchar *input,int *index,int *len);
//The result is not null terminated
uchar* derDecodeString(uchar *input,int *index,int *len);
int derDecodeInt(uchar *input,int *index);

#endif
#ifndef DER_H
#define DER_H

#include "globals.h"
#include "bigmaths.h"

int derEncodeBignum(uchar *result,int lenResult,bignum n,int lenN);
int derEncodeString(uchar *result,int lenResult,uchar *string,int lenString);
int derEncodeInt(uchar *result,int lenResult,int num);

bignum derDecodeBignum(uchar *input,int lenInput,int *index,int *len);
//The result is not null terminated
uchar* derDecodeString(uchar *input,int lenInput,int *index,int *len);
int derDecodeInt(uchar *input,int lenInput,int *index);

#endif
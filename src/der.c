#include "der.h"

#define DER_SEQUENCE 0x30
#define DER_INTEGER 0x02
#define DER_UTF8STRING 0x0C
#define DER_BITSTRING 0x03

int derEncodeBignum(uchar *result,bignum n,int lenN){
    result[0] = DER_BITSTRING;
    result[1] = lenN*4;
    for(int i=0;i<lenN*4;i++){
        //big endian
        result[i+2] = (n[i/4] >> ((3-(i&0x3))*8)) & 0xff;
    }
    return lenN*4+2;
}


int derEncodeString(uchar *result,uchar *string,int lenString){
    result[0] = DER_UTF8STRING;
    result[1] = lenString;
    for(int i=0;i<lenString;i++){
        result[i+2] = string[i];
    }
    return lenString+2;
}

int derEncodeInt(uchar *result,int num){
    result[0] = DER_INTEGER;
    result[1] = 4;
    for(int i=0;i<4;i++){
        //big endian
        result[2+i] = (num >> ((3-i)*8)) & 0xff;
    }
    return 6;
}


bignum derDecodeBignum(uchar *input,int *index,int *len){
    if(input[(*index)++]!=DER_BITSTRING){
        perror("derDecodeBignum: Input is not a bignum\n");
        exit(1);
    }
    *len = input[(*index)++];
    bignum result = calloc(*len,sizeof(uchar));
    if(!result){
        allocError();
    }
    const int startIndex = *index;
    for(;*index<startIndex+*len;(*index)++){
        //Big endian
        result[(*index-startIndex)>>2] |= (uint32_t) input[*index] << ((3-((*index-startIndex)&3))*8);
    }
    return result;
}


uchar* derDecodeString(uchar *input,int *index,int *len){
    if(input[(*index)++]!=DER_UTF8STRING){
        perror("derDecodeString: Input is not a string\n");
        exit(1);
    }
    *len = input[(*index)++];
    uchar *result = calloc(*len,sizeof(uchar));
    if(!result){
        allocError();
    }
    const int startIndex = *index;
    for(;*index<startIndex+*len;(*index)++){
        result[*index-startIndex] = input[*index];
    }
    return result;
}

int derDecodeInt(uchar *input,int *index){
    if(input[(*index)++]!=DER_INTEGER){
        perror("derDecodeInt: Input is not an int\n");
        exit(1);
    }
    const int len = input[(*index)++];
    if(len != 4){
        perror("derDecodeInt: Input is not an int\n");
        exit(1);
    }
    const int startIndex = *index;
    int result = 0;
    for(;*index<startIndex+len;(*index)++){
        //Big endian
        result |= (uint32_t) input[*index] << ((3-((*index-startIndex)&3))*8);
    }
    return result;
}


#ifndef DER_H
#define DER_H

#include "../other/globals.h"
#include "../bigmaths/bigmaths.h"


typedef enum derType{
    DER_INTEGER = 0x02,
    DER_BITSTRING = 0x03,
    DER_UTF8STRING = 0x0C,
    DER_SEQUENCE = 0x30,
} derType;

// A pointer is a array of size 1
typedef enum valueType{
    int32,
    chr, //a
    array,
    strct, //u
} valueType;

// Can be of type StructInfo or ArrayInfo
typedef uchar* Info; 

typedef struct StructInfo{
    size_t numFields;
    FieldInfo *fields;
    size_t *fieldOffsets;
} StructInfo;

typedef struct ArrayInfo{
    size_t valueSize; // Can be omitted for children of multi-dimensional arrays - inferred from parent
    valueType valueType; // Same as above
    Info *valueInfo; // Required if valueType is array or strct - can be ArrayInfo or StructInfo
} ArrayInfo;

typedef struct FieldInfo{
    valueType valueType;
    Info info; // Required if valueType is array or strct - can be ArrayInfo or StructInfo
} FieldInfo;

int derEncodeArray(uchar *result,size_t lenResult,uchar *arr,ArrayInfo info);
int derEncodeInt(uchar *result,size_t lenResult,int32_t num);
int derEncodeChar(uchar *result,size_t lenResult,uchar chr);
int derEncodeStruct(uchar *result,size_t lenResult,uchar *strct,StructInfo info);

uchar* derDecodeArray(uchar *input,int lenInput,int *index,int *len);
int derDecodeInt(uchar *input,int lenInput,int *index);
uchar derDecodeChar(uchar *input,int lenInput,int *index);
// Caller will need to free all fields
uchar* derDecodeStruct(uchar *input,int lenInput,int *index,uchar *strct);


#endif
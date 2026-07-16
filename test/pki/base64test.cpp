extern "C" {
    #include "../../src/pki/base64.h"
}
#include <gtest/gtest.h>


TEST(Base64Test,oneByteEncDec){
    uchar *msg = (uchar*)"a";
    String inp = {.data=msg,.lenData=1};
    String enc = base64Encode(inp,false);
    String res = base64Decode(enc,false);
    ASSERT_EQ(res.lenData,inp.lenData);
    ASSERT_EQ(res.data[0],msg[0]);
    free(enc.data);
    free(res.data);
}   


TEST(Base64Test,oneByteEncDecNullTerm){
    uchar *msg = (uchar*)"a";
    String inp = {.data=msg,.lenData=1};
    String enc = base64Encode(inp,false);
    String res = base64Decode(enc,true);
    ASSERT_EQ(res.lenData,2);
    ASSERT_EQ(res.data[0],msg[0]);
    ASSERT_EQ(res.data[1],'\0');
    free(enc.data);
    free(res.data);
}   


TEST(Base64Test,twoBytesEncDec){
    uchar *msg = (uchar*)"ab";
    String inp = {.data=msg,.lenData=2};
    String enc = base64Encode(inp,false);
    String res = base64Decode(enc,false);
    ASSERT_EQ(res.lenData,inp.lenData);
    for(int i=0;i<inp.lenData;i++){
        ASSERT_EQ(res.data[i],msg[i]);
    }
    free(enc.data);
    free(res.data);
}

TEST(Base64Test,threeBytesEncDec){
    uchar *msg = (uchar*)"abc";
    String inp = {.data=msg,.lenData=3};
    String enc = base64Encode(inp,false);
    String res = base64Decode(enc,false);
    ASSERT_EQ(res.lenData,inp.lenData);
    for(int i=0;i<inp.lenData;i++){
        ASSERT_EQ(res.data[i],msg[i]);
    }
    free(enc.data);
    free(res.data);
}

TEST(Base64Test,twentyBytesEncDec){
    uchar *msg = (uchar*)"12345678901234567890";
    String inp = {.data=msg,.lenData=20};
    String enc = base64Encode(inp,false);
    String res = base64Decode(enc,false);
    ASSERT_EQ(res.lenData,inp.lenData);
    for(int i=0;i<inp.lenData;i++){
        ASSERT_EQ(res.data[i],msg[i]);
    }
    free(enc.data);
    free(res.data);
}
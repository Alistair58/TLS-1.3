#include "base64.h"
#include <stdint.h>

//lenData should not include null terminator
//Output will not include a null terminator
String base64Encode(String inp){
    String result;
    int lenBits = inp.lenData*8;
    int lenPaddingBits = lenBits%6;
    int len6BitChunksBits = lenBits+lenPaddingBits;
    int num6BitChunks = len6BitChunksBits / 6;
    //add a '=' for each 2 padding bits
    int lenPaddingChars = lenPaddingBits/2;
    int lenResultBytes = num6BitChunks + lenPaddingChars;

    result.data = calloc(lenResultBytes,sizeof(uchar));
    result.lenData = lenResultBytes;

    for(int i=0;i<num6BitChunks;i++){
        int inputIndex = i*6/8;
        uchar inputChunk0 = inp.data[inputIndex];
        uchar inputChunk1 = (inputIndex>=inp.lenData)?0:inp.data[inputIndex+1];

        // i=0 -> input[0] & 0b11111100 >> 2 | input[1] & 0b00000000 >> 2
        // i=1 -> input[0] & 0b00000011 << 4 | input[1] & 0b11110000 >> 4
        // i=2 -> input[1] & 0b00001111 << 2 | input[2] & 0b11000000 >> 6
        // i=3 -> input[2] & 0b00111111 << 0 | input[3] & 0b00000000 >> 8
        // i=4 -> input[3] & 0b11111100 >> 2 | input[4] & 0b00000000 >> 2
        // i=5 -> input[3] & 0b00000011 | input[4] & 0b11110000 

        //There might be a nicer way of doing this
        //It's sort of a pattern but not a very nice one
        
        uint8_t chunk6Bits;
        switch(i%4){
            case 0:
                chunk6Bits = (inputChunk0 & 0b11111100) >> 2;
                break;
            case 1:
                chunk6Bits = (inputChunk0 & 0b00000011) << 4 | (inputChunk1 & 0b11110000) >> 4;
                break;
            case 2:
                chunk6Bits = (inputChunk0 & 0b00001111) << 2 | (inputChunk1 & 0b11000000) >> 6;
                break;
            case 3:
                chunk6Bits = (inputChunk0 & 0b00111111);
                break;
        }
        if(chunk6Bits<26){
            result.data[i] = 'A'+chunk6Bits;
        }
        else if(chunk6Bits<52){
            result.data[i] = 'a'+(chunk6Bits-26);
        }
        else if(chunk6Bits<62){
            result.data[i] = '0'+(chunk6Bits-52);
        }
        else if(chunk6Bits==62) result.data[i] = '+';
        else if(chunk6Bits==63) result.data[i] = '/';
    }
    //Padding
    for(int i=0;i<lenPaddingChars;i++){
        result.data[num6BitChunks+i] = '=';
    }

    return result;
}

//lenInput should not include a null terminator
//The result will not have a null terminator
String base64Decode(String inp){
    String result;
    int lenBits = inp.lenData*8;
    //Calculate the real length of the message
    int lenPaddingBits = 0;
    for(int i=inp.lenData-1;i>=0;i--){
        if(inp.data[i]=='='){
            lenPaddingBits+=2;
            //6 for the '=' and 2 for the actual padding
            lenBits-=8;
        }
        else break;
    }    
    int lenResultBytes = (lenBits/8)*6/8;

    result.data = calloc(lenResultBytes,sizeof(uchar));
    result.lenData = lenResultBytes;

    int lenNonPaddedInput = inp.lenData - lenPaddingBits/2;
    for(int inputIndex=0;inputIndex<lenNonPaddedInput;inputIndex++){
        
        int outputIndex = inputIndex*6/8;
        uchar b64Value = inp.data[inputIndex];
        if(b64Value >= 'A' && b64Value <= 'Z'){
            b64Value -= 'A';
        }
        else if(b64Value >= 'a' && b64Value <= 'z'){
            b64Value = b64Value-'a'+26;
        }
        else if(b64Value >= '0' && b64Value <= '9'){
            b64Value = b64Value-'0'+52;
        }
        else if(b64Value == '+'){
            b64Value = 62;
        }
        else if(b64Value == '/'){
            b64Value = 63;
        }
        else{
            fprintf(stderr,"base64Decode: Invalid base64 character at position: %d\n",inputIndex);
            exit(1);
        }

        // result[0] = input[0] << 2 | (input[1] & 0b00110000) >> 4
        // result[1] = (input[1] & 0b00001111) << 4 | (input[2] & 0b00111100) >> 2  
        // result[2] = (input[2] & 0b00000011) << 6 | (input[3] & 0b00111111) 
        // result[3] = input[4] << 2 | (input[5] & 0b00110000) >> 4

        if((inputIndex&3) == 0){ //inputIndex%4 == 0
            result.data[outputIndex] = b64Value << 2;
        }
        else if(outputIndex%3 == 0){ //&& inputIndex % 3 == 1
            result.data[outputIndex] |= (b64Value & 0b00110000) >> 4;
            if(outputIndex+1<lenResultBytes){
                result.data[outputIndex+1] = (b64Value & 0b00001111) << 4;
            }
        }
        else if(outputIndex%3 == 1){
            result.data[outputIndex] |= (b64Value & 0b00111100) >> 2; 
            if(outputIndex+1<lenResultBytes){
                result.data[outputIndex+1] = (b64Value & 0b00000011) << 6;
            }
        }
        else{ //outputIndex%3 == 2
            if((inputIndex&1) == 0){ //inputIndex%4 == 2
                result.data[outputIndex] = (b64Value & 0b00000011) << 6;
            }
            else{ //inputIndex%4 == 3
                result.data[outputIndex] |= (b64Value & 0b00111111);
            }
        }
    }
    return result;
}



#include <stdlib.h>
#include <math.h>
#include <string.h>
typedef unsigned char byte;

void aesEncrypt(unsigned long *key,int keyLen,unsigned long *data, unsigned long* dest);
void keyExpansion(unsigned long *key, int keyLen,byte **keyExp, int keyExpLen);
int numRounds = 14; // called Nr in paper
int numCols = 8; //called Nb 
int numRows = 4; //constant for all block sizes

//Everything is represented as byte (just an unsigned char) and arrays of these represent 
//what are called "words"/"4-byte vectors" in the AES paper
void aesEncrypt(unsigned long* key,int keyLen,unsigned long *data, unsigned long* dest){
    
    int keyExpLen = numCols * (numRounds+1);
    byte **keyExp = (byte**) calloc(keyExpLen,sizeof(byte*));
    keyExpansion(key,keyLen,keyExp,keyExpLen);
    byte **state = (byte**) calloc(numCols, sizeof(byte*));
    
    for(int i=0; i<numCols;i++){ //Put data into the state
        byte col[] = {((byte) data[i]>>24),((byte)data[i]>>16),((byte)data[i]>>8),((byte)data[i])};
        state[i] = col;
    }
    addRoundKey(state,keyExp); //First round is just the key
    for(int i=1 ; i<numRounds ; i++ ) round(state,keyExp,i) ;
    finalRound(state,keyExp);
 
}

void keyExpansion(unsigned long *key, int keyLen,byte **keyExp, int keyExpLen){
    for(int i = 0; i < keyLen; i++){
        byte keyExp4Bytes[] = {((byte) key[i]>>24),((byte)key[i]>>16),((byte)key[i]>>8),((byte)key[i])};
        memcpy(keyExp[i],keyExp4Bytes,4*sizeof(byte)); //TODO Won't work as pointer is uninitialised
    }
    for(int i = keyLen; i < numCols * (numRounds + 1); i++){
        byte *temp = (byte*) calloc(4,sizeof(byte));
        memcpy(temp,keyExp[i-1],4*sizeof(byte));
        if(i% keyLen == 0){ //If a multiple of keyLen do some mixing
            byte *rotated = rotate(temp);
            byte roundConst[] = {pow(2,((i/keyLen)-1)),0,0,0};//RC[i] = x^(i-1)
            for(int j=0;j<4;j++){
                temp[j] = subByte(rotated[j]) ^ roundConst[j];
            }
        }
        else if(i%keyLen == 4){
            for(int j=0;j<4;j++){
                temp[j] = subByte(temp[j]);
            }
        }

        for(int j=0;j<4;j++){
            keyExp[i][j] =  keyExp[i-keyLen][j] ^ temp[j];
        }
    }
 
}
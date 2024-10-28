#include <stdlib.h>
#include <string.h>
#include <math.h>
typedef unsigned char byte;

void aesEncrypt(unsigned long *key,int keyLen,unsigned long *data, unsigned long* dest);
void keyExpansion(unsigned long *key, int keyLen,byte **keyExp, int keyExpLen);
byte* rotate(byte *inp);
void initialiseByteSubTable();
//byte multiplicativeInverse(byte inp);
void addRoundKey(byte **state, byte **keyExp);
void aesRound(byte **state, byte **keyExp,int i);
void finalRound(byte **state, byte **keyExp);
byte subByte(byte inp);
byte polynomialModMult(byte x, byte y);
byte xtime(byte inp);

int numRounds = 14; // called Nr in paper
int numCols = 8; //called Nb 
int numRows = 4; //constant for all block sizes
bool byteSubTableInitialised = false;
byte **byteSubTable;


//Everything is represented as byte (just an unsigned char) and arrays of these represent 
//what are called "words"/"4-byte vectors" in the AES paper
void aesEncrypt(unsigned long* key,int keyLen,unsigned long *data, unsigned long* dest){
    if(!byteSubTableInitialised) initialiseByteSubTable();
    int keyExpLen = numCols * (numRounds+1);
    byte **keyExp = (byte**) calloc(keyExpLen,sizeof(byte*));
    byte **state = (byte**) calloc(numCols, sizeof(byte*));
    for(int i=0;i<((keyExpLen>numCols)?keyExpLen:numCols);i++){//Initialise pointers for 4-byte vectors
        if(i<keyExpLen){
            keyExp[i] = (byte*) calloc(4,sizeof(byte)); 
        }
        if(i<numCols){
            state[i] = (byte*) calloc(4,sizeof(byte)); //Initialise pointers for 4-byte vectors
            byte col[] = {((byte) data[i]>>24),((byte)data[i]>>16),((byte)data[i]>>8),((byte)data[i])};
            memcpy(state[i],col,4*sizeof(byte));
        }
    }
    keyExpansion(key,keyLen,keyExp,keyExpLen);
    addRoundKey(state,keyExp); //First round is just the key
    for(int i=1 ; i<numRounds ; i++ ) aesRound(state,keyExp,i) ;
    finalRound(state,keyExp);

    //memcpy to dest
    for(int i=0;i<((keyExpLen>numCols)?keyExpLen:numCols);i++){//Initialise pointers for 4-byte vectors
        if(i<keyExpLen){
            free(keyExp[i]); 
        }
        if(i<numCols){
            free(state[i]);
        }
    }
    free(keyExp);free(state);
 
}

void keyExpansion(unsigned long *key, int keyLen,byte **keyExp, int keyExpLen){
    for(int i = 0; i < keyLen; i++){
        byte keyExp4Bytes[] = {((byte) key[i]>>24),((byte)key[i]>>16),((byte)key[i]>>8),((byte)key[i])};
        memcpy(keyExp[i],keyExp4Bytes,4*sizeof(byte)); 
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
            free(rotated);
        }
        else if(i%keyLen == 4){
            for(int j=0;j<4;j++){
                temp[j] = subByte(temp[j]);
            }
        }

        for(int j=0;j<4;j++){
            keyExp[i][j] =  keyExp[i-keyLen][j] ^ temp[j];
        }
        free(temp);
    }
 
}

byte* rotate(byte *inp){
    byte *result = (byte*) calloc(4,sizeof(byte));
    for(int i=0;i<4;i++){
        result[i] = inp[(i-1)%4];
    }
    return result;
}

/*byte multiplicativeInverse(byte inp){
    int r1 = inp;
    int r2 = 0x11b;
    int s1 = 1;
    int s2,quotient,temp = 0;
    while(r2 != 0){
        quotient = r1 / r2;
        temp = r1;
        r1 = r2;
        r2 = temp - quotient *r2; //Just a modulo but quicker as we already have the quotient so this is quicker
        temp = s1;
        s1 = s2;
        s2 = temp - quotient *s2;
    } 
    return s1 % 0x11b; //Non-negativity
}*/

byte subByte(byte inp){
    return inp;
}

void finalRound(byte **state, byte **keyExp){

}

void initialiseByteSubTable(){

}
void addRoundKey(byte **state, byte **keyExp){

}
void aesRound(byte **state, byte **keyExp,int i){

}

byte polynomialModMult(byte x, byte y){
    int product = (int) (x&1)?x:0;
    int bit = 0;
    byte r = x;
    for(int i=1;i<8;i++){
        bit = (y >> i) & 1;
        r = xtime(r);
        if(bit){
            product ^= r;
        }
    }
    return product;
}

byte xtime(byte inp){ //Multiply by a single x as in paper
    int product;
    product = (int) inp << 1;
    if(product>>8 & 1){
        product ^= 0x1b;
    }
    return (byte) product;
}

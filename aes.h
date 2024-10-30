#include <stdlib.h>
#include <string.h>
#include <math.h>
typedef unsigned char byte;

void aesEncrypt(unsigned long *key,int keyLen,unsigned long *data, unsigned long* dest);
void keyExpansion(unsigned long *key, int keyLen,byte **keyExp, int keyExpLen);
byte* rotate(byte *inp);
void addRoundKey(byte **state, byte **keyExp,int i);
void aesRound(byte **state, byte **keyExp,int i);
void finalRound(byte **state, byte **keyExp);
byte polynomialModMult(byte x, byte y);
byte xtime(byte inp);
byte polynomialMSBDiv(int x, int y);

int numRounds = 14; // called Nr in paper
int numCols = 8; //called Nb 
int numRows = 4; //constant for all block sizes
byte sBox[] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};


//Everything is represented as byte (just an unsigned char) and arrays of these represent 
//what are called "words"/"4-byte vectors" in the AES paper
//state is stored as an array of columns
void aesEncrypt(unsigned long* key,int keyLen,unsigned long *data, unsigned long* dest){
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
    addRoundKey(state,keyExp,0); //First round is just the key
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
                temp[j] = sBox[rotated[j]] ^ roundConst[j];
            }
            free(rotated);
        }
        else if(i%keyLen == 4){
            for(int j=0;j<4;j++){
                temp[j] = sBox[temp[j]];
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




void finalRound(byte **state, byte **keyExp){

}

void addRoundKey(byte **state, byte **keyExp,int roundNum){
    for(int i=0;i<numCols;i++){
        for(int j=0;j<numRows;j++){
            state[i][j] ^= keyExp[roundNum*numCols + i][j];
        }
    }

}
void aesRound(byte **state, byte **keyExp,int roundNum){
    byteSub(state);
    shiftRow(state);
    mixColumn(state);
    addRoundKey(state,keyExp,roundNum);
}

void byteSub(byte **state){
    for(int i=0;i<numCols;i++){
        for(int j=0;j<numRows;j++){
            state[i][j] = sBox[state[i][j]];
        }
    }
}

void mixColumn(byte **state){
    byte c[] = {2,1,1,3}; //little endian
    for(int i=0;i<numCols;i++){
        vectorModMult(c,state[i],state[i]);
    }
}

void shiftRow(byte **state){
    byte prevRow1 = state[numCols-1][1];
    byte prevRow2 = state[numCols-][1];
    byte prevRow3 = state[numCols-1][1];
    for(int i=0;i<numCols;i++){
        state[i][1] = 
    }
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

void vectorModMult(byte *col1, byte *col2,byte *dest){ //Polynomial mult mod x^4 +1 from paper
    dest[0] = polynomialModMult(col1[0],col2[0]) ^
              polynomialModMult(col1[3],col2[1]) ^
              polynomialModMult(col1[2],col2[2]) ^
              polynomialModMult(col1[1],col2[3]); 
    dest[1] = polynomialModMult(col1[1],col2[0]) ^
              polynomialModMult(col1[0],col2[1]) ^
              polynomialModMult(col1[3],col2[2]) ^
              polynomialModMult(col1[2],col2[3]); 
    dest[2] = polynomialModMult(col1[2],col2[0]) ^
              polynomialModMult(col1[1],col2[1]) ^
              polynomialModMult(col1[0],col2[2]) ^
              polynomialModMult(col1[3],col2[3]); 
    dest[3] = polynomialModMult(col1[3],col2[0]) ^
              polynomialModMult(col1[2],col2[1]) ^
              polynomialModMult(col1[1],col2[2]) ^
              polynomialModMult(col1[0],col2[3]); 
}

byte xtime(byte inp){ //Multiply by a single x as in paper
    int product;
    product = (int) inp << 1;
    if(product>>8 & 1){
        product ^= 0x1b;
    }
    return (byte) product;
}

/*byte polynomialMSBDiv(int x, int y){ //x/y and the MSB of the quotient 
    int result = 0;
    if(y>x) return 0;
    for(int i=0;i<9;i++){ //9 instead of 8 for 0x11b
        if(y>>(8-i) & 1){
            int j;  //Can be modified to give full quotient by adding a loop here
            for(j=0;j<9;j++){
                if(x>>(8-j) & 1) break;
            }
            x ^= y<<(i-j);
            result ^= 1<<(i-j);
            break;
        }
        
    }
    return (byte) result;
}*/
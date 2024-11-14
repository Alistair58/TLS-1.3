#include <stdlib.h>
#include <string.h>
#include <math.h>
typedef unsigned char byte;

const int numRounds =  14; // called Nr in paper
const int numCols = 8; //called Nb 
const int numRows = 4; //constant for all block sizes
const int keyLen = 8; // Number of 4-byte vectors long the key is
const int keyExpLen = 120; //numCols * (numRounds+1);

void aesEncrypt(unsigned long *key,unsigned long *data, unsigned long* dest);
void aesDecrypt(unsigned long *key,unsigned long *data, unsigned long* dest);
void keyExpansion(unsigned long *key,byte keyExp[keyExpLen][4]);
void invKeyExpansion(unsigned long *key,byte invKeyExp[keyExpLen][4]);
byte* rotate(byte *inp);
void addRoundKey(byte state[numCols][4], byte keyExp[keyExpLen][4],int roundNum);
void aesRound(byte state[numCols][4], byte keyExp[keyExpLen][4],int roundNum);
void finalRound(byte state[numCols][4], byte keyExp[keyExpLen][4]);
void invAesRound(byte state[numCols][4], byte invKeyExp[keyExpLen][4],int roundNum);
void invFinalRound(byte state[numCols][4], byte invKeyExp[keyExpLen][4]);
byte polynomialModMult(byte x, byte y);
byte xtime(byte inp);
void vectorModMult(byte *col1, byte *col2,byte *dest);
void byteSub(byte state[numCols][4]);
void invByteSub(byte state[numCols][4]);
void mixColumn(byte state[numCols][4]);
void invMixColumn(byte state[numCols][4]);
void shiftRow(byte state[numCols][4]);
void invShiftRow(byte state[numCols][4]);
byte rCon(byte i);


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
byte invSBox[256] = {
    0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB,
    0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB,
    0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,
    0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25,
    0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92,
    0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
    0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06,
    0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B,
    0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
    0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E,
    0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B,
    0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
    0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,
    0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF,
    0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D
};

//AES 256 


//Everything is represented as byte (just an unsigned char) and arrays of these represent 
//what are called "words"/"4-byte vectors" in the AES paper
//Words are stored as little endian
//state is stored as an array of columns (words)

void aesEncrypt(unsigned long* key,unsigned long *data, unsigned long* dest){
    byte keyExp[120][4] = {{0}}; //Even though keyExpLen is const it needs a number to initialise array
    byte state[8][4] = {{0}};
    for(int i=0;i<numCols;i++){//Initialise state
        byte col[] = {((byte)data[i]),((byte)(data[i]>>8)),((byte)(data[i]>>16)),((byte) (data[i]>>24))}; //little endian
        memcpy(state[i],col,4*sizeof(byte));
    }
    keyExpansion(key,keyExp);
    addRoundKey(state,keyExp,0); //First round is just the key
    for(int i=1 ; i<numRounds ; i++ ) aesRound(state,keyExp,i) ;
    finalRound(state,keyExp);

    for(int i=0;i<numCols;i++){//Copy to dest
        dest[i] = (unsigned long) state[i][0] | (unsigned long) state[i][1]<<8 
                | (unsigned long) state[i][2]<<16 | (unsigned long) state[i][3]<<24;
    }
}

void aesDecrypt(unsigned long* key,unsigned long *data, unsigned long* dest){
    byte invKeyExp[120][4] = {{0}}; //Even though keyExpLen is const it needs a number to initialise array
    byte state[8][4] = {{0}};
    for(int i=0;i<numCols;i++){//Initialise state
        byte col[] = {((byte)(data[i])),((byte)(data[i]>>8)),((byte)(data[i]>>16)),((byte) (data[i]>>24))};
        memcpy(state[i],col,4*sizeof(byte));
    }
    invKeyExpansion(key,invKeyExp);
    addRoundKey(state,invKeyExp,numRounds); //First round is just the key
    for(int i=numRounds-1 ; i>0;i--) invAesRound(state,invKeyExp,i) ;
    invFinalRound(state,invKeyExp);

    for(int i=0;i<numCols;i++){//Copy to dest
        dest[i] = (unsigned long) state[i][0] | (unsigned long) state[i][1]<<8 
                | (unsigned long) state[i][2]<<16 | (unsigned long) state[i][3]<<24;
    }
}

void invKeyExpansion(unsigned long *key,byte invKeyExp[keyExpLen][4]){
    keyExpansion(key,invKeyExp);
    byte d[] = {0x0E,0x09,0x0D,0x0B}; //MixColumn but for our funny sized key expansion
    for(int i=1;i<numRounds;i++){
        vectorModMult(d,invKeyExp[i],invKeyExp[i]);
    }
}

void keyExpansion(unsigned long *key,byte keyExp[keyExpLen][4]){
    for(int i = 0; i < keyLen; i++){
        byte keyExp4Bytes[] = {((byte) (key[i])),((byte) (key[i]>>8)),((byte) (key[i]>>16)),((byte)(key[i]>>24))}; //keeping with little endian
        memcpy(keyExp[i],keyExp4Bytes,4*sizeof(byte)); 
    }
    for(int i = keyLen; i < numCols * (numRounds + 1); i++){
        byte *temp = (byte*) calloc(4,sizeof(byte));
        if(!temp){
            perror("Calloc error during key expansion");
            exit(1);
        }
        memcpy(temp,keyExp[i-1],4*sizeof(byte));
        if(i% keyLen == 0){ //If a multiple of keyLen do some mixing
            byte *rotated = rotate(temp);
            byte roundConst[] = {rCon(i/keyLen),0,0,0};//RC[i] = x^(i-1) in GF 2^8
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
    int index = 0;
    if(!result){
        perror("Calloc error during rotate");
        exit(1);
    }
    for(int i=0;i<4;i++){
        result[i] = inp[(i+1)%4];
    }
    return result;
}




void finalRound(byte state[numCols][4],byte keyExp[keyExpLen][4]){
    byteSub(state);
    shiftRow(state);
    addRoundKey(state,keyExp,numRounds);
}

void invFinalRound(byte state[numCols][4],byte invKeyExp[keyExpLen][4]){
    invByteSub(state);
    invShiftRow(state);
    addRoundKey(state,invKeyExp,0);
}

void addRoundKey(byte state[numCols][4],byte keyExp[keyExpLen][4],int roundNum){
    for(int i=0;i<numCols;i++){
        for(int j=0;j<numRows;j++){
            state[i][j] ^= keyExp[roundNum*numCols + i][j];
        }
    }

}
void aesRound(byte state[numCols][4],byte keyExp[keyExpLen][4],int roundNum){
    byteSub(state);
    shiftRow(state);
    mixColumn(state);
    addRoundKey(state,keyExp,roundNum);
}

void invAesRound(byte state[numCols][4],byte invKeyExp[keyExpLen][4],int roundNum){
    invByteSub(state);
    invShiftRow(state);
    invMixColumn(state);
    addRoundKey(state,invKeyExp,roundNum);
}

void byteSub(byte state[numCols][4]){
    for(int i=0;i<numCols;i++){
        for(int j=0;j<numRows;j++){
            state[i][j] = sBox[state[i][j]];
        }
    }
}

void invByteSub(byte state[numCols][4]){
    for(int i=0;i<numCols;i++){
        for(int j=0;j<numRows;j++){
            state[i][j] = invSBox[state[i][j]];
        }
    }
}

void mixColumn(byte state[numCols][4]){
    byte c[] = {2,1,1,3}; //little endian
    for(int i=0;i<numCols;i++){
        vectorModMult(c,state[i],state[i]);
    }
}

void invMixColumn(byte state[numCols][4]){
    byte d[] = {0x0E,0x09,0x0D,0x0B}; //little endian
    for(int i=0;i<numCols;i++){
        vectorModMult(d,state[i],state[i]);
    }
}

void shiftRow(byte state[numCols][4]){
    byte row1[4] = {0};
    byte row2[4] = {0};
    byte row3[4] = {0};
    for(int i=numCols-1;i>-1;i--){
        row1[i] = state[(i+1)%numCols][1];
        row2[i] = state[(i+3)%numCols][2];
        row3[i] = state[(i+4)%numCols][3];
    }
    for(int i=0;i<numCols;i++){
        state[i][1] = row1[i];
        state[i][2] = row2[i];
        state[i][3] = row3[i];
    }
}

void invShiftRow(byte state[numCols][4]){
    byte row1[4] = {0};
    byte row2[4] = {0};
    byte row3[4] = {0};
    for(int i=0;i<numCols;i++){
        row1[i] = state[(i-1)%numCols][1];
        row2[i] = state[(i-3)%numCols][2];
        row3[i] = state[(i-4)%numCols][3];
    }
    for(int i=0;i<numCols;i++){
        state[i][1] = row1[i];
        state[i][2] = row2[i];
        state[i][3] = row3[i];
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

void vectorModMult(byte *inp1, byte *inp2,byte *dest){ //Polynomial mult mod x^4 +1 from paper
    byte col1[4] = {0}; //in case destination is the same as one of the inputs
    byte col2[4] = {0};
    memcpy(col1,inp1,4*sizeof(byte));
    memcpy(col2,inp2,4*sizeof(byte));
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

byte rCon(byte i){
    byte product = 1;
    for(int j=1;j<i;j++){ //Not constant time - but does not leak any information
        product = xtime(product);
    }
    return product;
}
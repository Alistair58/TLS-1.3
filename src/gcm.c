#include "gcm.h"
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

static void gf256Mult(uint32_t *x,uint32_t *y, uint32_t *out);
static void increment(uint32_t *iv);
static void plaintextXOR(uint32_t *key, uchar *plaintext, int blockNum);
static void bigNumXOR(uint32_t *inp1, uint32_t *inp2,int length);
static void bigNumXORDest(uint32_t *dest,uint32_t *inp1, uint32_t *inp2,int length);
static void polyMult(uint32_t *x,uint32_t *y,uint32_t *out);
static void modGf256(uint32_t *dividend,uint32_t *out);
static void polyLeftShift(uint32_t *inp,int shiftBy,uint32_t *out);

//GCM supports 128 bit block sizes but my AES implementation uses 256 bit blocks and so I've altered
//this GCM to use 256 bit blocks which means that multiplication now takes place in GF(2^256) instead of GF(2^128)

//If there is no destination parameter, it will store the result in the first argument

//Encrypt and decrypt with this function
bool gcm(uchar *plaintext, int lenPlaintext, uint32_t *key,gcmResult *dest){
    uint32_t *temp = (uint32_t*) calloc(8,sizeof(uint32_t));
    uint32_t *prevBlock = (uint32_t*) calloc(8,sizeof(uint32_t));
    uint32_t *h = (uint32_t*) calloc(8,sizeof(uint32_t));
    uint32_t **blocks; 
    uint32_t *iv = (uint32_t*) calloc(8,sizeof(uint32_t));//iv changes but the gcmResult one doesn't
    if(!iv) goto allocError;
    if(!dest->iv){ //If the user hasn't supplied an iv
        dest->iv = (uint32_t*) calloc(8,sizeof(uint32_t));
        if(!dest->iv ) goto allocError;
        randomNumber(iv,8,NULL);
    }
    else{
        memcpy(iv,dest->iv,8*sizeof(uint32_t));
    }
    
    int numBlocks = ceil((float)lenPlaintext/32);
    if(!temp || !h || !iv || !prevBlock){
        goto allocError;
    }
    aesEncrypt(key,temp,h); //temp will start off with all zeros
    
    dest->ciphertext = (uchar*) calloc(32*numBlocks,sizeof(uchar)); 
    if(!dest->ciphertext){
        goto allocError;
    }
    memcpy(dest->iv,iv,8*sizeof(uint32_t));
    blocks = (uint32_t**) calloc(numBlocks,sizeof(uint32_t*));
    for(int i=0;i<numBlocks;i++){
        blocks[i] = (uint32_t*) calloc(8,sizeof(uint32_t));
        if(!blocks[i]) goto allocError;
    }
    bool reverse = dest->tag!=0; //If we are in reverse we already have a tag to "work towards"
    for(int i=0;i<numBlocks;i++){
        increment(iv);
        aesEncrypt(key,iv,temp);//encrypt current IV
        plaintextXOR(temp,plaintext,i);//xor with plaintext block to get encrypted block
        memcpy(blocks[i],temp,8*sizeof(uint32_t));//Save encrypted block
        if(reverse){//If we are going in reverse we need to XOR the plaintext (the ciphertext we were given) in order to compute the same tag
            for(int j=0;j<8;j++){//32 bit chunks = 4 chars
                temp[j] = ((plaintext[(i<<5) + 4*j]<<24) | (plaintext[(i<<5) + 4*j + 1]<<16)
                            | (plaintext[(i<<5) + 4*j + 2]<<8) | plaintext[(i<<5) + 4*j + 3]);
            }
        }
        bigNumXOR(temp,prevBlock,8); //prevBlock will be all 0s on first iteration which works
        gf256Mult(temp,h,prevBlock);//multiply by h
    }
    prevBlock[7] ^= numBlocks; //XOR in length of authenticated data (none) and number of blocks
    gf256Mult(prevBlock,h,prevBlock);
    aesEncrypt(key,dest->iv,temp); //counter 0
    bigNumXOR(prevBlock,temp,8);
    if(!dest->tag){
        dest->tag = (uint32_t*) calloc(8,sizeof(uint32_t));
        if(!dest->tag) goto allocError;
        memcpy(dest->tag,prevBlock,8*sizeof(uint32_t));
    } 
    else{
        //Checking tag against what was received
        printf("\nReceived tag: ");
        for(int i=0;i<8;i++){
            printf("%lu",dest->tag[i]);
        }
        printf("\nRecomputed tag: ");
        for(int i=0;i<8;i++){
            printf("%lu",prevBlock[i]);
        }
        for(int i=0;i<8;i++){
            if(dest->tag[i] != prevBlock[i]){
                printf("\nGCM tag does not match.\nMessage has been altered during transit");
                return false;
            }
        }
    }
    
    for(int i=0;i<numBlocks;i++){ //256 bit blocks = 32 chars
        for(int j=0;j<8;j++){//32 bit chunks = 4 chars
            dest->ciphertext[(i<<5) + j*4    ] = (uchar) (blocks[i][j]>>24);
            dest->ciphertext[(i<<5) + j*4 + 1] = (uchar) ((blocks[i][j]>>16) & 0xff);
            dest->ciphertext[(i<<5) + j*4 + 2] = (uchar) ((blocks[i][j]>>8)  & 0xff);
            dest->ciphertext[(i<<5) + j*4 + 3] = (uchar) ((blocks[i][j])     & 0xff);
        }
    }

    goto freeMem;
    allocError:
        if(temp) free(temp);
        if(h) free(h);
        if(iv) free(iv);
        if(prevBlock) free(prevBlock);
        if(blocks){
            for(int i=0;i<numBlocks;i++){
                if(blocks[i]) free(blocks[i]);
            }
            free(blocks);
        }
        perror("GCM calloc error");
        exit(1);
    
    freeMem:
        if(temp) free(temp);
        if(h) free(h);
        if(iv) free(iv);
        if(prevBlock) free(prevBlock);
        if(blocks){
            for(int i=0;i<numBlocks;i++){
                if(blocks[i]) free(blocks[i]);
            }
            free(blocks);
        }
    return true;
    
}   

void gf256Mult(uint32_t *x,uint32_t *y, uint32_t *out){
    uint32_t *temp = (uint32_t*) calloc(16,sizeof(uint32_t));
    if(!temp){
        perror("GCM calloc error");
        exit(1);
    }
    polyMult(x,y,temp); //8,8 -> 16
    modGf256(temp,out); //16 -> 8
    free(temp);
}

void modGf256(uint32_t *dividend,uint32_t *out){ //16 long input
    uint32_t divisor[] = {0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0x0000425}; //x^256 + x^10 + x^5 + x^2 + 1
    // from https://shiftleft.com/mirrors/www.hpl.hp.com/techreports/98/HPL-98-135.pdf
    //Note sure if its insecure or anything, its just irreducible and works in GF(2^256)
    uint32_t *temp = (uint32_t*) calloc(16,sizeof(uint32_t));
    if(!temp){
        if(temp) free(temp);
        perror("GCM calloc error");
        exit(1);
    }
    for(int i=0;i<256;i++){ //the dividend is 512 bits long but we stop when we get in the finite field (<divisor)
        if(dividend[i>>5]>>(31 - (i&31)) & 1){
            polyLeftShift(divisor,(255-i),temp);
            bigNumXOR(dividend,temp,16);
        }
    }
    memcpy(out,&dividend[8],8*sizeof(uint32_t)); //the remainder
    free(temp);

}

void polyMult(uint32_t *x,uint32_t *y,uint32_t *out){ //8 long * 8 long -> 16 long
    uint32_t *product = (uint32_t*) calloc(16,sizeof(uint32_t));
    uint32_t *temp = (uint32_t*) calloc(16,sizeof(uint32_t));
    if(!temp || !product){
        if(temp) free(temp);
        if(product) free(product);
        perror("GCM calloc error");
        exit(1);
    }
    for(int i=255;i>=0;i--){ //LSB to MSB
        polyLeftShift(x,(255-i),temp); //do it anyway - constant time complexity
        if(y[i>>5]>>(31 - (i&31)) & 1) bigNumXOR(product,temp,16);
        else bigNumXOR(temp,temp,16); //completely useless - constant time
    }
    memcpy(out,product,16*sizeof(uint32_t));
    free(product);
    free(temp);
}

void polyLeftShift(uint32_t *inp,int shiftBy,uint32_t *out){ //8 long in 16 long out
    uint32_t carry = 0;
    uint64_t shifted = 0;
    
    int wholeMoves = shiftBy/32; //whole chunk shifts
    int partMoves = shiftBy&31; //smaller shifts (<32 bit shifts)
    for(int i=0;i<8;i++){
        out[8+i-wholeMoves] = inp[i];
    }
    for(int i=15;i>=0;i--){
        shifted = out[i]<<partMoves;
        out[i] =  (shifted&0xffffffff) ^ carry;
        carry = shifted >> 32;
    }
}

void increment(uint32_t *iv){
    for(int i=7;i>=0;i--){
        if(iv[i] == ULONG_MAX) iv[i] = 0;
        else{
            iv[i]++;
            return;
        }
    }
    perror("GCM increment overflow error");
    exit(1);
}

void plaintextXOR(uint32_t *key, uchar *plaintext, int blockNum){
    for(int i=0;i<8;i++){ //Order doesn't matter - this is MSB to LSB and start of text to end
        key[i] ^= ((plaintext[blockNum*32 + 4*i]<<24) + (plaintext[blockNum*32 + 4*i + 1]<<16)
                            + (plaintext[blockNum*32 + 4*i + 2]<<8) + plaintext[blockNum*32 + 4*i + 3]);
    }
}

void bigNumXOR(uint32_t *inp1, uint32_t *inp2,int length){
    for(int i=0;i<length;i++){
        inp1[i] ^= inp2[i];
    }
}
void bigNumXORDest(uint32_t *dest,uint32_t *inp1, uint32_t *inp2,int length){
    for(int i=0;i<length;i++){
        dest[i] = inp1[i] ^ inp2[i];
    }
}
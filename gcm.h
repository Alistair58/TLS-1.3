#include "random.h"
#include "aes.h"

struct gcmResult{
    unsigned long *iv;
    unsigned long **blocks;
    unsigned long *tag;
};
void gcm(char *plaintext, int lenPlaintext, unsigned long *key, gcmResult *dest);
void gf256Mult(unsigned long *x,unsigned long *y, unsigned long *out);
void increment(unsigned long *iv);
void plaintextXOR(unsigned long *key, char *plaintext, int blockNum);
void bigNumXOR(unsigned long *inp1, unsigned long *inp2,int length);
void polyMult(unsigned long *x,unsigned long *y,unsigned long *out);
void modGf256(unsigned long *dividend,unsigned long *out);
void polyShift(unsigned long *inp,int shiftBy,unsigned long *out);

//GCM supports 128 bit block sizes but my AES implementation uses 256 bit blocks and so I've altered
//this GCM to use 256 bit blocks which means that multiplication now takes place in GF(2^256) instead of GF(2^128)

//If there is no destination parameter, it will store the result in the first argument

void gcm(char *plaintext, int lenPlaintext, unsigned long *key, gcmResult *dest){
    unsigned long *temp = (unsigned long*) calloc(8,sizeof(unsigned long));
    unsigned long *prevBlock = (unsigned long*) calloc(8,sizeof(unsigned long));
    unsigned long *h = (unsigned long*) calloc(8,sizeof(unsigned long));
    unsigned long *iv = (unsigned long*) calloc(8,sizeof(unsigned long));
    randomNumber(iv,8,NULL);
    int numBlocks = ceil((float)lenPlaintext/256);
    if(!temp || !h || !iv || !prevBlock){
        goto callocError;
    }
    aesEncrypt(key,temp,h); //temp will start off with all zeros
    dest->iv = (unsigned long*) calloc(8,sizeof(unsigned long));
    if(!dest->iv){
        goto callocError;
    }
    memcpy(dest->iv,iv,8*sizeof(unsigned long));
    dest->blocks = (unsigned long**) calloc(numBlocks,sizeof(unsigned long*));
    for(int i=0;i<numBlocks;i++){
        dest->blocks[i] = (unsigned long*) calloc(8,sizeof(unsigned long));
        if(!dest->blocks[i]) goto callocError;
    }
    for(int i=0;i<numBlocks;i++){
        increment(iv);
        aesEncrypt(key,iv,temp);//encrypt current IV
        plaintextXOR(temp,plaintext,i);//xor with plaintext block to get encrypted block
        memcpy(dest->blocks[i],temp,8*sizeof(unsigned long));//Save encrypted block
        bigNumXOR(temp,prevBlock,8); //prevBlock will be all 0s on first iteration which works
        gf256Mult(temp,h,prevBlock);//multiply by h
    }
    prevBlock[7] ^= numBlocks; //XOR in length of authenticated data (none) and number of blocks
    gf256Mult(prevBlock,h,prevBlock);
    aesEncrypt(key,dest->iv,temp); //counter 0
    bigNumXOR(prevBlock,temp,8);
    memcpy(dest->tag,prevBlock,8*sizeof(unsigned long));

    callocError:
        goto freeMem;
        perror("GCM calloc error");
        exit(1);
    
    freeMem:
        if(temp) free(temp);
        if(h) free(h);
        if(iv) free(iv);
        if(prevBlock) free(prevBlock);
        if(dest->iv) free(dest->iv);
        if(dest->tag) free(dest->tag);
        if(dest->blocks){
            for(int i=0;i<numBlocks;i++){
                if(dest->blocks[i]) free(dest->blocks[i]);
            }
            free(dest->blocks);
        }

    
}   

void gf256Mult(unsigned long *x,unsigned long *y, unsigned long *out){
    unsigned long *temp = (unsigned long*) calloc(16,sizeof(unsigned long));
    if(!temp){
        perror("GCM calloc error");
        exit(1);
    }
    polyMult(x,y,temp);
    modGf256(temp,out);
    free(temp);
}

void modGf256(unsigned long *dividend,unsigned long *out){ //16 long input
    unsigned long divisor[] = {0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0x0000425}; //x^256 + x^10 + x^5 + x^2 + 1
    // from https://shiftleft.com/mirrors/www.hpl.hp.com/techreports/98/HPL-98-135.pdf
    //Note sure if its insecure or anything, its just irreducible and works in GF(2^256)
    unsigned long *temp = (unsigned long*) calloc(16,sizeof(unsigned long));
    if(!temp){
        if(temp) free(temp);
        perror("GCM calloc error");
        exit(1);
    }
    for(int i=0;i<256;i++){ //the dividend is 512 bits long but we stop when we get in the finite field (<divisor)
        if(dividend[i>>5]>>(31 - (i&31)) & 1){
            polyShift(divisor,(255-i),temp);
            bigNumXOR(dividend,temp,16);
        }
    }
    memcpy(out,dividend,8*sizeof(unsigned long)); //the remainder
    free(temp);

}

void polyMult(unsigned long *x,unsigned long *y,unsigned long *out){
    unsigned long *product = (unsigned long*) calloc(16,sizeof(unsigned long));
    unsigned long *temp = (unsigned long*) calloc(16,sizeof(unsigned long));
    if(!temp || !product){
        if(temp) free(temp);
        if(product) free(product);
        perror("GCM calloc error");
        exit(1);
    }
    for(int i=255;i>=0;i++){ //LSB to MSB
        polyShift(x,(255-i),temp); //do it anyway - constant time complexity
        if(y[i>>5]>>(31 - (i&31)) & 1) bigNumXOR(product,temp,16);
        else bigNumXOR(temp,temp,16); //completely useless - constant time
    }
    free(product);
    free(temp);
}

void polyShift(unsigned long *inp,int shiftBy,unsigned long *out){ //8 long in 16 long out
    unsigned long prev = 0;
    
    int wholeMoves = shiftBy/32; //whole chunk shifts
    int partMoves = shiftBy&31; //smaller shifts (<32 bit shifts)
    for(int i=0;i<8;i++){
        out[i+shiftBy] = inp[shiftBy];
    }
    for(int i=0;i<16;i++){
        out[i] = (out[i]<<partMoves) ^ prev;
    }
}

void increment(unsigned long *iv){
    for(int i=7;i>=0;i--){
        if(iv[i] == (1<<64 - 1)) iv[i] = 0;
        else{
            iv[i]++;
            return;
        }
    }
    perror("GCM increment overflow error");
    exit(1);
}

void plaintextXOR(unsigned long *key, char *plaintext, int blockNum){
    for(int i=0;i<8;i++){ //Order doesn't matter - this is MSB to LSB and start of text to end
        key[i] ^= (plaintext[blockNum*16 + 4*i] + plaintext[blockNum*16 + 4*i + 1]
                            + plaintext[blockNum*16 + 4*i + 2] + plaintext[blockNum*16 + 4*i + 3]);
    }
}

void bigNumXOR(unsigned long *inp1, unsigned long *inp2,int length){
    for(int i=0;i<length;i++){
        inp1[i] ^= inp2[i];
    }
}
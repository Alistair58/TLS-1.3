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
void XOR256(unsigned long *inp1, unsigned long *inp2);


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
        XOR256(temp,prevBlock); //prevBlock will be all 0s on first iteration which works
        gf256Mult(temp,h,prevBlock);//multiply by h
    }

    callocError:
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
        perror("GCM calloc error");
        exit(1);

    
}   

void gf256Mult(unsigned long *x,unsigned long *y, unsigned long *out){

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

void XOR256(unsigned long *inp1, unsigned long *inp2){
    for(int i=0;i<8;i++){
        inp1[i] ^= inp2[i];
    }
}
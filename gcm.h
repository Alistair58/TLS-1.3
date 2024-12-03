#include "random.h"
#include "aes.h"

struct gcmResult{
    unsigned long *IV;
    unsigned long **blocks;
    unsigned long *tag;
};
void gcmEncrypt(char *plaintext, int lenPlaintext, unsigned long *key, gcmResult *dest);
void gf128Mult(unsigned long *x,unsigned long *y, unsigned long *out);

void gcmEncrypt(char *plaintext, int lenPlaintext, unsigned long *key, gcmResult *dest){
    unsigned long *blank = (unsigned long*) calloc(8,sizeof(unsigned long));
    unsigned long *h = (unsigned long*) calloc(8,sizeof(unsigned long));
    if(!blank || !h){
        if(blank) free(blank);
        if(h) free(h);
        perror("GCM calloc error");
        exit(1);
    }
    aesEncrypt(key,blank,h);
    int numBlocks = ceil((float)lenPlaintext/256);
    for(int i=0;i<numBlocks;i++){
        //increment IV and store somewhere else
        //need an increment function
        //encrypt current IV
        //xor with plaintext block to get encrypted block
        //write encrypted block to blocks
        //xor with previous result - start with blank at i=0
        //multiply by h with function
        //store as previous result
    }
    
}   

void gf128Mult(unsigned long *x,unsigned long *y, unsigned long *out){

}
#include "x25519.h"
#include <stdlib.h>
#include <string.h>
#include "bigmaths.h"
#include "structs.h"
#include "globals.h"

//Based on: https://martin.kleppmann.com/papers/curve25519.pdf
//Which is based on TweetNaCl by Daniel J. Bernstein
void swapPoints(bignum a1,bignum a2,int lenA, int bit);
void montgomeryLadder(bignum x1,bignum x2,bignum z1,bignum z2,bignum x);

bignum X25519(uint32_t *p,uint32_t *inpQ){
    int bit;
    bignum x1 = (uint32_t*) calloc(8,sizeof(uint32_t)); bignum x2 = (uint32_t*) calloc(8,sizeof(uint32_t));
    bignum z1 = (uint32_t*) calloc(8,sizeof(uint32_t)); bignum z2 = (uint32_t*) calloc(8,sizeof(uint32_t));
    bignum pSub2 = (uint32_t*) calloc(8,sizeof(uint32_t));bignum q = (uint32_t*) calloc(8,sizeof(uint32_t));
    bignum result = (bignum) calloc(8,sizeof(uint32_t));
    memcpy(x2,p,8*sizeof(uint32_t));memcpy(pSub2,curve25519Params.p,8*sizeof(uint32_t));
    memcpy(q,inpQ,8*sizeof(uint32_t));
    pSub2[7] -=2; 
    q[7] &= 0xfffffff8; //q[0] & 11111 ... 1000 - set last 3 bits = 0 so it is multiple of 8 - small group confinement
    q[0] = (q[0] & 0x7fffffff) | 0x40000000; //Remove MSB and set next bit to 1 so it runs in constant time
    if(!x1 || !x2 || !z1 || !z2 || !pSub2 || !q || !result){
        free(x1); free(x2);
        free(z1); free(z2);
        free(pSub2); free(q);
        free(result);
        allocError();
    }
    x1[7] = z2[7] = 1;
    for(int i=1; i<256;i++){ //Start at bit 255       
        bit = (q[i >> 5] >> (31-(i & 31))) & 1; //i>>5 divides by 32 and tell us which chunk we want; i&31 shifts by the chunk i mod 32
        swapPoints(x1,x2,8,bit); //Swap if bit is 1
        swapPoints(z1,z2,8,bit); //Run anyway for constant time
        montgomeryLadder(x1,x2,z1,z2,p);
        
        swapPoints(x1,x2,8,bit); //Swap back
        swapPoints(z1,z2,8,bit); 

    }
    bigNumBitModInv(z1,8,pSub2,8,result,8,255,19);
    bigNumBitModMult(result,8,x1,8,result,8,255,19);

    free(x1); free(x2);
    free(z1); free(z2);
    free(pSub2); free(q);
    return result;
}

void swapPoints(bignum a1,bignum a2,int lenA, int bit){
    uint32_t t,i,c;
    c = ~(bit-1); //Not (bit-1) this means that 1 is true and any other value is false
    for(i=0;i<lenA;i++){
        t = c & (a1[i] ^ a2[i]); // t is 0 if bit is 0 and otherwise it is a1 XOR a2
        a1[i] ^= t; //If t is 0, value is the same, else a1 XOR a1 XOR a2 = a2
        a2[i] ^= t; //If t is 0, value is the same, else a2 XOR a1 XOR a2 = a1

    }
}

void montgomeryLadder(bignum x1,bignum x2,bignum z1,bignum z2,bignum x){   
    //If there is a performance issue, these bignums could be reused between function calls
    bignum r[14]; //As it is 0-indexed, the names of the variables are -1 what they are in the paper
    for(int i=0;i<14;i++){
        r[i] = (bignum) calloc(8,sizeof(uint32_t));
    }
    bigNumBitModAdd(x1,8,z1,8,r[0],8,255,19); //x1 + z1 
    bigNumModSub(x1,8,z1,8,r[1],8,curve25519Params.p,8); //x1- z1
    bigNumBitModAdd(x2,8,z2,8,r[2],8,255,19); //x2 + z2
    bigNumModSub(x2,8,z2,8,r[3],8,curve25519Params.p,8); //x2- z2
    bigNumBitModMult(r[0],8,r[0],8,r[4],8,255,19); //(x1+z1)^2
    bigNumBitModMult(r[1],8,r[1],8,r[5],8,255,19); //(x1-z1)^2
    bigNumBitModMult(r[1],8,r[2],8,r[6],8,255,19);//(x1-z1)(x2+z2)
    bigNumBitModMult(r[0],8,r[3],8,r[7],8,255,19);//(x1+z1)(x2-z2)
    bigNumBitModAdd(r[6],8,r[7],8,r[8],8,255,19); // 2(x1x2 − z1z2)
    bigNumModSub(r[6],8,r[7],8,r[9],8,curve25519Params.p,8); // 2(x1z2 -x2z1)
    bigNumBitModMult(r[9],8,r[9],8,r[10],8,255,19);// 4(x1z2 -x2z1)^2
    bigNumModSub(r[4],8,r[5],8,r[11],8,curve25519Params.p,8); // 4x1z1
    bigNumBitModMultByLittle(r[11],8,121665,r[12],8,255,19); // (A-2)x1z1
    bigNumBitModAdd(r[12],8,r[4],8,r[13],8,255,19); // x1^2 + Ax1z1 + z1^2
    bigNumBitModMult(r[11],8,r[13],8,z1,8,255,19); //4x1z1 (x1^2 + Ax1z1 + z1^2)
    bigNumBitModMult(r[4],8,r[5],8,x1,8,255,19); //(x1^2 − z1^2)^2
    bigNumBitModMult(r[10],8,x,8,z2,8,255,19); //4x(x1z2-x2z1)^2
    bigNumBitModMult(r[8],8,r[8],8,x2,8,255,19); //4(x1x2 - z1z2)^2

    for(int i=0;i<14;i++){
        free(r[i]);
    }
    
}
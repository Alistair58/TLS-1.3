#include <stdlib.h>
#include <string.h>
#include "bigmaths.h"


uint32_t* X25519(uint32_t* p,uint32_t *q);
void swapPoints(uint32_t *a1, uint32_t *a2,int lenA, int bit);
void montgomeryLadder(uint32_t *x1, uint32_t *x2, uint32_t *z1, uint32_t *z2,uint32_t *x);




uint32_t* X25519(uint32_t *p,uint32_t *inpQ){
    int bit;
    uint32_t *x1,*x2,*z1,*z2,*pSub2,*q;
    x1 = (uint32_t*) calloc(8,sizeof(uint32_t)); x2 = (uint32_t*) calloc(8,sizeof(uint32_t));
    z1 = (uint32_t*) calloc(8,sizeof(uint32_t)); z2 = (uint32_t*) calloc(8,sizeof(uint32_t));
    pSub2 = (uint32_t*) calloc(8,sizeof(uint32_t));q = (uint32_t*) calloc(8,sizeof(uint32_t));
    memcpy(x2,p,8*sizeof(uint32_t));memcpy(pSub2,curve25519Params.p,8*sizeof(uint32_t));
    memcpy(q,inpQ,8*sizeof(uint32_t));
    pSub2[7] -=2; 
    q[7] &= 0xfffffff8; //q[0] & 11111 ... 1000 - set last 3 bits = 0 so it is multiple of 8 - small group confinement
    q[0] = (q[0] & 0x7fffffff) | 0x40000000; //Remove MSB and set next bit to 1 so it runs in constant time
    if(!x1 || !x2 || !z1 || !z2 || !pSub2){
        perror("\nMalloc error during X25519");
        exit(1);
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
    uint32_t *invZ1 = bigNumBitModInv(z1,8,pSub2,8,8,255,19);
    
    uint32_t *result = bigNumBitModMult(invZ1,8,x1,8,8,255,19);
    free(invZ1);free(x1);
    free(x2);free(z1);free(z2);free(pSub2);free(q);
    return result;
}

void swapPoints(uint32_t *a1, uint32_t *a2,int lenA, int bit){
    uint32_t t,i,c;
    c = ~(bit-1); //Not (bit-1) this means that 1 is true and any other value is false
    for(i=0;i<lenA;i++){
        t = c & (a1[i] ^ a2[i]); // t is 0 if bit is 0 and otherwise it is a1 XOR a2
        a1[i] ^= t; //If t is 0, value is the same, else a1 XOR a1 XOR a2 = a2
        a2[i] ^= t; //If t is 0, value is the same, else a2 XOR a1 XOR a2 = a1

    }
}

void montgomeryLadder(uint32_t *x1, uint32_t *x2, uint32_t *z1, uint32_t *z2,uint32_t *x){   
    uint32_t *r1 = bigNumBitModAdd(x1,8,z1,8,8,255,19); //x1 + z1 
    uint32_t *r2 = bigNumModSub(x1,8,z1,8,8,curve25519Params.p,8); //x1- z1
    uint32_t *r3 = bigNumBitModAdd(x2,8,z2,8,8,255,19); //x2 + z2
    uint32_t *r4= bigNumModSub(x2,8,z2,8,8,curve25519Params.p,8); //x2- z2
    uint32_t *r5= bigNumBitModMult(r1,8,r1,8,8,255,19); //(x1+z1)^2
    uint32_t *r6 = bigNumBitModMult(r2,8,r2,8,8,255,19); //(x1-z1)^2
    uint32_t *r7 = bigNumBitModMult(r2,8,r3,8,8,255,19);//(x1-z1)(x2+z2)
    uint32_t *r8 = bigNumBitModMult(r1,8,r4,8,8,255,19);//(x1+z1)(x2-z2)
    uint32_t *r9 = bigNumBitModAdd(r7,8,r8,8,8,255,19); // 2(x1x2 − z1z2)
    uint32_t *r10 = bigNumModSub(r7,8,r8,8,8,curve25519Params.p,8); // 2(x1z2 -x2z1)
    uint32_t *r11 = bigNumBitModMult(r10,8,r10,8,8,255,19);// 4(x1z2 -x2z1)^2
    uint32_t *r12 = bigNumModSub(r5,8,r6,8,8,curve25519Params.p,8); // 4x1z1
    uint32_t *r13 = bigNumBitModMultByLittle(r12,8,121665,8,255,19); // (A-2)x1z1
    uint32_t *r14 = bigNumBitModAdd(r13,8,r5,8,8,255,19); // x1^2 + Ax1z1 + z1^2
    uint32_t *newZ1 = bigNumBitModMult(r12,8,r14,8,8,255,19); //4x1z1 (x1^2 + Ax1z1 + z1^2)
    uint32_t *newX1 = bigNumBitModMult(r5,8,r6,8,8,255,19); //(x1^2 − z1^2)^2
    uint32_t *newZ2 = bigNumBitModMult(r11,8,x,8,8,255,19); //4x(x1z2-x2z1)^2
    uint32_t *newX2 = bigNumBitModMult(r9,8,r9,8,8,255,19); //4(x1x2 - z1z2)^2

    memcpy(x1,newX1,8*sizeof(uint32_t));memcpy(x2,newX2,8*sizeof(uint32_t));
    memcpy(z1,newZ1,8*sizeof(uint32_t));memcpy(z2,newZ2,8*sizeof(uint32_t));
    free(r1);free(r2);free(r3);free(r4);free(r5);free(r6);free(r7);free(r8);free(r9);free(r10);free(r11);free(r12);free(r13);free(r14);
    free(newZ1);free(newX1);free(newZ2);free(newX2);
    
}
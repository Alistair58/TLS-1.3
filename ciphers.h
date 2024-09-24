#include <sysinfoapi.h>
#include "bigmaths.h"
int randomNumber(unsigned long *bigIntArr, int chunks,unsigned long *n);
unsigned long* X25519(unsigned long* p,unsigned long *q);
void swapPoints(unsigned long *a1, unsigned long *a2,int lenA, int bit);
void montgomeryLadder(unsigned long *x1, unsigned long *x2, unsigned long *z1, unsigned long *z2,unsigned long *x);


int randomNumber(unsigned long *bigIntArr, int chunks,unsigned long *n){  //A chunk is 32 bits
    int time = 1000;
    unsigned long long product; //Unsigned long would mean the mod would never work and would output 0
    int x;
    int y;
    POINT point;
    ULONGLONG tickCount = GetTickCount64();
    ULONGLONG targetTime = tickCount +time; //2.5 seconds later
    product = (tickCount) % (unsigned long)(pow(256,sizeof(unsigned long))-1); //mod 4 bytes
    int chunkWriteCount = 1;
    unsigned long mod;
    if(n==NULL) mod = pow(256,sizeof(unsigned long))-1; //Can only get up to 256^unsigned long -2
    else mod = n[0];
    while(chunkWriteCount<=chunks){
    
        GetCursorPos(&point);
        x = point.x;
        y = point.y;
        if(product){ //If not zero
            product = (product * x * y)% mod;
        }
        else{
            product += x + y;
        }
        if((targetTime - tickCount) <= time*((float)(chunks-chunkWriteCount)/(chunks+1))){ //+1 means chunks aren't written at start or end
            int timeProg = (int)(targetTime - tickCount);
            //TIME PROG PRINTS OUT ZERO
            printf("\ntime %d chunkWriteCount %d chunks+2 %d",time,chunkWriteCount,chunks+2);
            int timeNeeded = (int)time*((float)(chunks-chunkWriteCount+1)/(chunks+1));
            printf("\nchunkWriteCount-1 %d timeProgressed %d timeNeeded %d product %lu",chunkWriteCount-1,timeProg,timeNeeded,product);
            bigIntArr[chunkWriteCount-1] = product;
            chunkWriteCount ++;
            mod = pow(256,sizeof(unsigned long))-1;
            if(chunkWriteCount > chunks) break;
        }
        tickCount = GetTickCount64();
    }
    
    return 0;
}

unsigned long* X25519(unsigned long *p,unsigned long *q){
    int bit;
    unsigned long *x1,*x2,*z1,*z2,*nSub2;
    x1 = calloc(8,sizeof(unsigned long)); x2 = calloc(8,sizeof(unsigned long));
    z1 = calloc(8,sizeof(unsigned long)); z2 = calloc(8,sizeof(unsigned long));
    nSub2 = calloc(8,sizeof(unsigned long));
    memcpy(x2,p,8*sizeof(unsigned long));memcpy(nSub2,curve25519Params.n,8*sizeof(unsigned long));
    nSub2[7] &= 0xFFFFFFEB; //Manually sub 2;
    q[0] &= 0xfffffff8; //q[0] & 11111 ... 1000 - set last 3 bits = 0 so it is multiple of 8
    q[7] = (q[7] & 0x7fffffff) | 0x40000000; //Remove MSB and set next bit to 1 so it runs in constant time
    if(!x1 || !x2 || !z1 || !z2 || !nSub2){
        perror("\nMalloc error during X25519");
        exit(1);
    }
    x1[7] = z2[7] = 1;

    for(int i=1; i<256;i++){ //Start at bit 255       
        bit = (q[i >> 5] >> (i & 31)) & 1; //i>>5 divides by 32 and tell us which chunk we want; i&31 shifts by the chunk i mod 32
        swapPoints(x1,x2,8,bit); //Swap if bit is 1
        swapPoints(z1,z2,8,bit); //Run anyway for constant time
        printf("\ni: %d bit: %d",i,bit);
        printBigNum("X1: ",x1,8);
        printBigNum("X2: ",x2,8);
        printBigNum("z1: ",z1,8);
        printBigNum("z2: ",z2,8);
        montgomeryLadder(x1,x2,z1,z2,p);
        
        swapPoints(x1,x2,8,bit); //Swap back
        swapPoints(z1,z2,8,bit); 

    }
    printf("\n Done");
    unsigned long *invZ1 = bigNumModInv(z1,8,nSub2,8,8,255,19);
    printBigNum("Inverse ",invZ1,8);
    unsigned long *result = bigNumModMult(invZ1,8,x1,8,8,255,19);
    
    memcpy(p,result,8*sizeof(unsigned long));
    free(result);free(invZ1);free(x1);free(x2);free(z1);free(z2);free(nSub2);
    return p;
}

void swapPoints(unsigned long *a1, unsigned long *a2,int lenA, int bit){
    unsigned long t,i,c;
    c = ~(bit-1); //Not (bit-1) this means that 1 is true and any other value is false
    for(i=0;i<lenA;i++){
        t = c & (a1[i] ^ a2[i]); // t is 0 if bit is 0 and otherwise it is a1 XOR a2
        a1[i] ^= t; //If t is 0, value is the same, else a1 XOR a1 XOR a2 = a2
        a2[i] ^= t; //If t is 0, value is the same, else a2 XOR a1 XOR a2 = a1

    }
}

void montgomeryLadder(unsigned long *x1, unsigned long *x2, unsigned long *z1, unsigned long *z2,unsigned long *x){
     
    
    unsigned long *r1 = bigNumModAdd(x1,8,z1,8,8,255,19); //x1 + z1
    printBigNum("r1: ",r1,8);
    
    unsigned long *r2 = bigNumModSub(x1,8,z1,8,8,255,19,curve25519Params.p,8); //x1- z1
    printBigNum("r2: ",r2,8);
    unsigned long *r3 = bigNumModAdd(x2,8,z2,8,8,255,19); //x2 + z2
    printBigNum("r3: ",r3,8);
    unsigned long *r4= bigNumModSub(x2,8,z2,8,8,255,19,curve25519Params.p,8); //x2- z2
    printBigNum("r4: ",r4,8);
    unsigned long *r5= bigNumModMult(r1,8,r1,8,8,255,19); //(x1+z1)^2
    printBigNum("r5: ",r5,8);
    unsigned long *r6 = bigNumModMult(r2,8,r2,8,8,255,19); //(x1-z1)^2
    printBigNum("r6: ",r6,8);
    unsigned long *r7 = bigNumModMult(r2,8,r3,8,8,255,19);//(x1-z1)(x2+z2)
    printBigNum("r7: ",r7,8);
    unsigned long *r8 = bigNumModMult(r1,8,r4,8,8,255,19);//(x1+z1)(x2-z2)
    printBigNum("r8: ",r8,8);
    unsigned long *r9 = bigNumModAdd(r7,8,r8,8,8,255,19); // 2(x1x2 − z1z2)
    printBigNum("r9: ",r9,8);
    unsigned long *r10 = bigNumModSub(r7,8,r8,8,8,255,19,curve25519Params.p,8); // 2(x1z2 -x2z1)
    printBigNum("r10: ",r10,8);
    unsigned long *r11 = bigNumModMult(r10,8,r10,8,8,255,19);// 4(x1z2 -x2z1)^2
    printBigNum("r11: ",r11,8);
    unsigned long *r12 = bigNumModSub(r5,8,r6,8,8,255,19,curve25519Params.p,8); // 4x1z1
    printBigNum("r12: ",r12,8);
    unsigned long *r13 = bigNumModMultByLittle(r12,8,121665,8,255,19); // (A-2)x1z1
    printBigNum("r13: ",r13,8);
    unsigned long *r14 = bigNumModAdd(r13,8,r5,8,8,255,19); // x1^2 + Ax1z1 + z1^2
    printBigNum("r14: ",r14,8);
    unsigned long *newZ1 = bigNumModMult(r12,8,r14,8,8,255,19); //4x1z1 (x1^2 + Ax1z1 + z1^2)
    printBigNum("newZ1: ",newZ1,8);
    unsigned long *newX1 = bigNumModMult(r5,8,r6,8,8,255,19); //(x1^2 − z1^2)^2
    printBigNum("newX1: ",newX1,8);
    unsigned long *newZ2 = bigNumModMult(r11,8,x,8,8,255,19); //4x(x1z2-x2z1)^2
    printBigNum("newZ2: ",newZ2,8);
    unsigned long *newX2 = bigNumModMult(r9,8,r9,8,8,255,19); //4(x1x2 - z1z2)^2
    printBigNum("newX2: ",newX2,8);
    memcpy(x1,newX1,8*sizeof(unsigned long));memcpy(x2,newX2,8*sizeof(unsigned long));
    
    memcpy(z1,newZ1,8*sizeof(unsigned long));memcpy(z2,newZ2,8*sizeof(unsigned long));
    
    free(r1);free(r2);free(r3);free(r4);free(r5);free(r6);free(r7);free(r8);free(r9);free(r10);free(r11);free(r12);free(r13);free(r14);
    free(newZ1);free(newX1);free(newZ2);free(newX2);
    
}
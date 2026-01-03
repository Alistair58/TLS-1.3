#include "rsa.h"
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "random.h"

#define max(a,b) (a)>=(b) ? (a) : (b)

void extendedEuclidean(uint32_t exp,bignum totient,int lenTotient,bignum dest,int lenDest);
void montLadExp(bignum a,int lenA,bignum exp, int lenExp, bignum mod, int modLen,bignum dest, int lenDest);
bool millerRabin(bignum n,int lenN,bignum a,int lenA);

bool isPrime(bignum n,int lenN){ 
    if(bigNumCmpLittle(n,lenN,2) != GREATER_THAN){
        perror("isPrime: isPrime only works for n>2");
        exit(1);
    }
    //Check evens first
    if((n[lenN-1]&1)==0) return false;
    bignum nSub1 = (bignum) calloc(lenN,sizeof(uint32_t));
    bigNumSubLittle(n,lenN,1,nSub1,lenN);
    bignum a = calloc(lenN,sizeof(uint32_t));
    if(!a){
        allocError();
    }
    for(int i=0;i<30;i++){ //as MR is incorrect 1/4 of time, it is now incorrect 1 in 4^30 times (once every 36558901 years if it runs every ms)
        do{
            randomNumber(a,lenN,nSub1,0); //Fermats little theorem only works for 1<a<n-1
        }
        while(bigNumCmpLittle(a,lenN,0)==EQUAL);
        if(!millerRabin(n,lenN,a,lenN)){
            free(a);
            free(nSub1);
            return false;
        }
    }
    free(a);
    free(nSub1);
    //If n passed all the tests, very likely prime
    return true;
}

void montLadExp(bignum a,int lenA,bignum exp, int lenExp, bignum mod, int modLen,bignum dest, int lenDest){
    /*Square and always multiply
    Constant time
    e.g. 2^5
    5 = 101
    x1 = 2, x2 = 4
    1: skip first bit
    0: x2 = x1*x2 = 8, x1 = 2*2 = 4
    1: x1 = x1*x2 = 32, x2 = 8*8 = 64
    return 32 
    https://en.wikipedia.org/wiki/Exponentiation_by_squaring
    */  
    if(lenDest != modLen){
        perror("montLadExp: lenDest == modLen check failed\n");
        exit(1);
    }
    int lenLongest = max(lenA,modLen);
    bignum x1 = calloc(lenLongest,sizeof(uint32_t));
    bignum x2 = calloc(modLen,sizeof(uint32_t));
    //Buffers for saving unnecessary allocations inside bigNumModMult
    bignum sameOperandBuff = calloc(lenLongest,sizeof(uint32_t));
    int lenPreModBuff = lenLongest*2;
    bignum preModBuff = calloc(lenPreModBuff,sizeof(uint32_t));
    if(!x1 || !x2 || !sameOperandBuff || !preModBuff){
        free(x1); free(x2);
        free(sameOperandBuff); free(preModBuff);
        allocError();
    }
    memcpy(&x1[lenLongest-lenA],a,lenA*sizeof(uint32_t));
    
    //x1 = a
    //x2 = a^2
    bigNumModMultBuff(
        a,lenA,
        x1,lenLongest, //which holds a
        mod,modLen,
        preModBuff,lenPreModBuff,
        x2,modLen
    );
    bool started = false; //start when we reach the first 1 bit (as we have pre-set the first multiplication)
    for(int i=0;i<lenExp;i++){ //MSB to LSB chunks
        for(int j=0;j<32;j++){ //MSB to LSB inside of each chunk
            uint8 bit = (exp[i] >> (31-j))&1;
            if(started){
                if(bit){
                    bigNumModMultBuff(  
                        x1,lenLongest,
                        x2,modLen,
                        mod,modLen,
                        preModBuff,lenPreModBuff,
                        x1,lenLongest
                    );
                    memset(sameOperandBuff,0,(lenLongest-modLen)*sizeof(uint32_t));
                    memcpy(&sameOperandBuff[lenLongest-modLen],x2,modLen*sizeof(uint32_t));
                    bigNumModMultBuff(  
                        x2,modLen,
                        sameOperandBuff,lenLongest,
                        mod,modLen,
                        preModBuff,lenPreModBuff,
                        x2,modLen
                    );
                }
                else{
                    bigNumModMultBuff(  
                        x2,modLen,
                        x1,lenLongest,
                        mod,modLen,
                        preModBuff,lenPreModBuff,
                        x2,modLen
                    );
                    memcpy(sameOperandBuff,x1,lenLongest*sizeof(uint32_t));      
                    bigNumModMultBuff(  
                        x1,lenLongest,
                        sameOperandBuff,lenLongest,
                        mod,modLen,
                        preModBuff,lenPreModBuff,
                        x1,lenLongest
                    );
                }
            }
            else if(bit){
                started = true;
            }
            
        }
    }
    
    memcpy(dest,&x1[lenLongest-modLen],modLen*sizeof(uint32_t));
    free(x1); free(x2);
    free(sameOperandBuff); free(preModBuff);
}

//Single test case
bool millerRabin(bignum n,int lenN,bignum a,int lenA){
    //n is odd and so n-1 can be written as (2^s)*d where d is a positive integer 
    bignum nSub1 = (bignum) calloc(lenN,sizeof(uint32_t));
    bignum exp = (bignum) calloc(lenN,sizeof(uint32_t));
    bignum expRes = calloc(lenN,sizeof(uint32_t));
    if(!nSub1 || !exp || !expRes){
        free(nSub1);
        free(exp);
        free(expRes);
        allocError();
    }
    bigNumSubLittle(n,lenN,1,nSub1,lenN);
    memcpy(exp,nSub1,lenN*sizeof(uint32_t));
    int numShifts = 0;
    if(bigNumCmpLittle(exp,lenN,0)==EQUAL){
        perror("millerRabin: nSub1 cannot be 0");
        exit(1);
    }
    //Turn n-1 into d by shifting right s times
    while(!(exp[lenN-1] & 1)){
        bigNumRShift(exp,lenN,1,exp,lenN); //Keep shifting until it's odd
        numShifts++;
    }
    montLadExp(a,lenA,exp,lenN,n,lenN,expRes,lenN);
    //check if x = 1 mod n
    if(
        bigNumCmpLittle(expRes,lenN,1) == EQUAL ||
        bigNumCmp(expRes,lenN,nSub1,lenN) == EQUAL 
    ){
        free(nSub1);
        free(exp);
        free(expRes);
        return true;
    }
    for(int i=1;i<numShifts;i++){
        bigNumModMult(expRes,lenN,expRes,lenN,n,lenN,expRes,lenN);
         // a^(n-1/2^k) + 1 is a multiple of n -> satisfying Fermat's Little Theorem -> likely prime
        if(bigNumCmp(expRes,lenN,nSub1,lenN)==EQUAL){
            free(nSub1);
            free(exp);
            free(expRes);
            return true;
        }
        if(bigNumCmpLittle(expRes,lenN,1)==EQUAL){
            free(nSub1);
            free(exp);
            free(expRes);
            return false;
        }
    }
    free(nSub1);
    free(exp);
    free(expRes);
    return false; 
}

void encryptRSA(uchar *msg,int lenMsg,RSAKeyPair kp,bignum dest,int lenDest){
    if(lenDest != kp.publicKey.lenN){
        perror("encryptRSA: lenDest must be the length of the public key n");
    }
    int sizeDiff = sizeof(uint32_t)/sizeof(uchar); //yes I know it's 4
    //Making msgNum length lenMsgNum means that it could be < lenN
    //This means when you decrypt it, the first chunks may be zero and so you can't print it out
    //Making it lenN is easier
    bignum msgNum = calloc(kp.publicKey.lenN,sizeof(uint32_t));
    if(!msgNum){
        allocError();
    }
    //Putting the msg in starting at the LSB of msgNum means that we don't fail the < n check
    int j = kp.publicKey.lenN; //dencrements on first iteration
    for(int i=0;i<lenMsg;i++){
        int mod = i%sizeDiff;
        if(mod==0) j--;
        msgNum[j] |= (uint32_t) msg[i] << ((sizeDiff-1-mod)*8);
    }
    //RSA maths requirement
    if(bigNumCmp(msgNum,kp.publicKey.lenN,kp.publicKey.n,kp.publicKey.lenN) == GREATER_THAN){
        free(msgNum);
        perror("msg is too long for RSA encryption with this n\n");
        exit(1);
    }
    uint32_t eBigNum[1] = {kp.publicKey.e};
    montLadExp(msgNum,kp.publicKey.lenN,&kp.publicKey.e,1,kp.publicKey.n,kp.publicKey.lenN,dest,kp.publicKey.lenN);
    free(msgNum);
}

void extendedEuclidean(uint32_t exp,bignum totient,int lenTotient,bignum dest,int lenDest){
    if(lenDest!=lenTotient){
        perror("extendedEuclidean: destination must be same size as the totient");
        exit(1);
    }
    bignum r1 = (bignum) calloc(lenTotient,sizeof(uint32_t));
    bignum r2 = (bignum) calloc(lenTotient,sizeof(uint32_t));
    //dest is s1
    bignum s2 = (bignum) calloc(lenTotient,sizeof(uint32_t));
    bignum temp = (bignum) calloc(lenTotient,sizeof(uint32_t));
    bignum quotient = (bignum) calloc(lenTotient,sizeof(uint32_t));
    bignum bigTemp = (bignum) calloc(2*lenTotient,sizeof(uint32_t));
    if(!r1 || !r2 || !s2 || !temp || !quotient || !bigTemp){
        free(r1); 
        free(r2);
        free(s2);
        free(temp);
        free(quotient);
        free(bigTemp);
        allocError();
    }
    memcpy(r1,totient,lenTotient*sizeof(uint32_t));
    r2[lenTotient-1] = exp;
    memset(dest,0,lenDest*sizeof(uint32_t));
    //s1 (dest) stays at 0
    s2[lenTotient-1] = 1;
    
    while(bigNumCmpLittle(r2,lenTotient,0) != EQUAL){
        //quotient = r1//r2
        //(r1, r2) = (r2, r1 - quotient *r2) //Just a modulo but quicker as we already have the quotient so this is quicker
        //(s1, s2) = (s2, s1 - quotient *s2) 

        bigNumDiv(  r1,lenTotient,
                    r2,lenTotient,
                    quotient,lenTotient);
        memcpy(temp,r2,lenTotient*sizeof(uint32_t));
        
        bigNumMult( quotient,lenTotient,
                    r2,lenTotient,
                    bigTemp,2*lenTotient);
        //We know that quotient*r2 will fit in lenTotient

        bigNumSub(r1,lenTotient,&bigTemp[lenTotient],lenTotient,r1,lenTotient);
        memcpy(r2,r1,lenTotient*sizeof(uint32_t));
        memcpy(r1,temp,lenTotient*sizeof(uint32_t));
        
        //We must stay non-negative
        //The standard euclidean algorithm allows s1 and s2 to be negative but we don't
        //quotient*s2
        bigNumModMult(  quotient,lenTotient,
                        s2,lenTotient,
                        totient,lenTotient,
                        temp,lenTotient);
        //totient-quotient*s2
        bigNumSub(  totient,lenTotient,
                    temp,lenTotient,
                    temp,lenTotient);
        //(s1+totient-quotient*2)%totient
        bigNumModAdd(   dest,lenTotient,
                        temp,lenTotient,
                        totient,lenTotient,
                        temp,lenTotient);
        memcpy(dest,s2,lenTotient*sizeof(uint32_t));
        memcpy(s2,temp,lenTotient*sizeof(uint32_t));
    }
    bigNumModMult(&exp,1,dest,lenDest,totient,lenTotient,r2,lenTotient);
    free(r1); free(r2);
    free(s2); free(temp);
    free(quotient); free(bigTemp);
}

void decryptRSA(bignum encryptedMessage,int lenEM,RSAKeyPair kp,uchar *dest,int lenDest){
    int sizeDiff = sizeof(uint32_t)/sizeof(uchar); //yes I know it's 4
    int lenMsg = lenEM*sizeDiff;
    if(lenMsg != lenDest){
        perror("decryptRSA: lenDest does not match the length of the decrypted message");
        exit(1);
    }
    bignum pSub1 = (bignum) calloc(kp.privateKey.lenP,sizeof(uint32_t));
    bignum qSub1 = (bignum) calloc(kp.privateKey.lenQ,sizeof(uint32_t));
    bignum totient = (bignum) calloc(kp.publicKey.lenN,sizeof(uint32_t));
    bignum d = (bignum) calloc(kp.publicKey.lenN,sizeof(uint32_t));
    bignum decryptedNum = (bignum) calloc(kp.publicKey.lenN,sizeof(uint32_t));
    if(!pSub1 || !qSub1 || !totient || !d || !decryptedNum){
        free(pSub1); free(qSub1);
        free(totient); free(d);
        free(decryptedNum);
        allocError();
    }
    bigNumSubLittle(kp.privateKey.p,kp.privateKey.lenP,1,pSub1,kp.privateKey.lenP); 
    bigNumSubLittle(kp.privateKey.q,kp.privateKey.lenQ,1,qSub1,kp.privateKey.lenQ);
    bigNumMult(pSub1,kp.privateKey.lenP,qSub1,kp.privateKey.lenQ,totient,kp.publicKey.lenN);
    extendedEuclidean(kp.publicKey.e,totient,kp.publicKey.lenN,d,kp.publicKey.lenN);
    montLadExp( encryptedMessage,lenEM,
                d,kp.publicKey.lenN,
                kp.publicKey.n,kp.publicKey.lenN,
                decryptedNum,kp.publicKey.lenN);
    int j = kp.publicKey.lenN; //decrements on first iteration
    //Opposite of encryption encoding
    for(int i=0;i<lenMsg;i++){
        int mod = i%sizeDiff;
        if(mod==0) j--;
        dest[i]  =  (decryptedNum[j] >> ((sizeDiff-1-mod)*8)) & 0xff;
    }
    free(pSub1); free(qSub1);
    free(totient); free(d);
    free(decryptedNum);
}

//numBits is the size of the public key n
RSAKeyPair generateKeys(int numBits){
    RSAKeyPair kp;
    int publicKeyLen = numBits/(8*sizeof(uint32_t));
    int privateKeyLen = publicKeyLen/2;
    kp.privateKey.p = calloc(privateKeyLen,sizeof(uint32_t));
    kp.privateKey.q = (bignum) calloc(privateKeyLen,sizeof(uint32_t));
    kp.publicKey.n = (bignum) calloc(publicKeyLen,sizeof(uint32_t));
    kp.publicKey.lenN = publicKeyLen;
    kp.privateKey.lenP = privateKeyLen;
    kp.privateKey.lenQ = privateKeyLen;
    if(!kp.privateKey.p || !kp.privateKey.q || !kp.publicKey.n){
        free(kp.privateKey.p);
        free(kp.privateKey.q);
        free(kp.publicKey.n);
        allocError();
    }
    do{
        randomNumber(kp.privateKey.p,privateKeyLen,NULL,0);
    }
    while(!isPrime(kp.privateKey.p,privateKeyLen));
    
    do{
        randomNumber(kp.privateKey.q,privateKeyLen,NULL,0);
    }
    while(!isPrime(kp.privateKey.q,privateKeyLen));
   
    bigNumMult(
        kp.privateKey.p,privateKeyLen,
        kp.privateKey.q,privateKeyLen,
        kp.publicKey.n,publicKeyLen
    );
    kp.publicKey.e = 65537; //Good number for RSA; 65537 == 2**16+1
    return kp;
}


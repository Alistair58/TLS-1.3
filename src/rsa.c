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
        printf("i: %d\n",i);
        do{
            randomNumber(a,lenN,nSub1,0); //Fermats little theorem only works for 1<a<n-1
        }
        while(bigNumCmpLittle(a,lenN,0)==EQUAL);
        printBigNum("a: ",a,lenN);
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
        perror("\nmontLadExp: lenDest == modLen check failed");
        exit(1);
    }
    int lenLongest = max(lenA,modLen);
    bignum x1 = calloc(lenLongest,sizeof(uint32_t));
    bignum x2 = calloc(modLen,sizeof(uint32_t));
    if(!x1 || !x2){
        free(x1);
        free(x2);
        allocError();
    }
    memcpy(&x1[lenLongest-lenA],a,lenA*sizeof(uint32_t));
    
    bigNumModMult(a,lenA,a,lenA,mod,modLen,x2,modLen);
    bool started = false; //start when we reach the first 1 bit (as we have pre-set the first multiplication)
    for(int i=0;i<lenExp;i++){ //MSB to LSB chunks
        for(int j=0;j<32;j++){ //MSB to LSB inside of each chunk
            uint8 bit = (exp[i] >> (31-j))&1;
            if(started){
                if(bit){
                    bigNumModMult(  x1,lenLongest,
                                    x2,modLen,
                                    mod,modLen,
                                    x1,lenLongest);
                    bigNumModMult(  x2,modLen,
                                    x2,modLen,
                                    mod,modLen,
                                    x2,modLen);
                }
                else{
                    bigNumModMult(  x2,modLen,
                                    x1,lenLongest,
                                    mod,modLen,
                                    x2,modLen);
                    bigNumModMult(  x1,lenLongest,
                                    x1,lenLongest,
                                    mod,modLen,
                                    x1,lenLongest);
                }
            }
            else if(bit){
                started = true;
            }
            
        }
    }
    
    memcpy(dest,&x1[lenLongest-modLen],modLen*sizeof(uint32_t));
    free(x1);
    free(x2);
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
    printBigNum("nSub1: ",nSub1,lenN);
    printf("Num shifts: %d\n",numShifts);
    montLadExp(a,lenA,exp,lenN,n,lenN,expRes,lenN);
    //check if x = 1 mod n
    if(
        bigNumCmpLittle(expRes,lenN,1) == EQUAL ||
        bigNumCmp(expRes,lenN,nSub1,lenN) == EQUAL 
    ){
        printf("Passed on the primality check with no squares\n");
        free(nSub1);
        free(exp);
        free(expRes);
        return true;
    }
    for(int i=1;i<numShifts;i++){
        bigNumModMult(expRes,lenN,expRes,lenN,n,lenN,expRes,lenN);
         // a^(n-1/2^k) + 1 is a multiple of n -> satisfying Fermat's Little Theorem -> likely prime
        if(bigNumCmp(expRes,lenN,nSub1,lenN)==EQUAL){
            printf("Passed the primality check on iteration: %d\n",i);
            free(nSub1);
            free(exp);
            free(expRes);
            return true;
        }
        if(bigNumCmpLittle(expRes,lenN,1)==EQUAL){
            printf("Failed the primality check on iteration: %d\n",i);
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


bignum encryptRSA(uchar *msg,int lenMsg,RSAKeyPair kp){
    int sizeDiff = sizeof(uint32_t)/sizeof(uchar); //yes I know it's 4
    int lenMsgNum = ceil((float)lenMsg*1/sizeDiff);
    bignum msgNum = calloc(lenMsgNum,sizeof(uint32_t));
    bignum result = calloc(kp.publicKey.lenN,sizeof(uint32_t));
    if(!msgNum || !result){
        free(msgNum);
        free(result);
        allocError();
    }
    int j = -1; //increments on first iteration
    for(int i=0;i<lenMsg;i++){
        int mod = i%sizeDiff;
        if(mod==0) j++;
        msgNum[j] |= (uint32_t) msg[i] << ((sizeDiff-1-mod)*8);
    }
    //RSA maths requirement
    if(bigNumCmp(msgNum,lenMsgNum,kp.publicKey.n,kp.publicKey.lenN) == GREATER_THAN){
        printf("Len msg num: %d\n",lenMsgNum);
        free(msgNum);
        free(result);
        perror("msg is too long for RSA encryption with this N\n");
        exit(1);
    }
    uint32_t eBigNum[1] = {kp.publicKey.e};
    montLadExp(msgNum,lenMsgNum,&kp.publicKey.e,1,kp.publicKey.n,kp.publicKey.lenN,result,kp.publicKey.lenN);
    free(msgNum);
    return result;
}

void extendedEuclidean(uint32_t exp,bignum totient,int lenTotient,bignum dest,int lenDest){
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
    r1[lenTotient-1] = exp;
    memcpy(r2,totient,lenTotient*sizeof(uint32_t));
    memset(dest,0,lenDest*sizeof(uint32_t));
    dest[lenTotient-1] = 1;
    //s2 stays at 0
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
        memcpy(r2,&bigTemp[lenTotient],lenTotient*sizeof(uint32_t));
        memcpy(temp,r2,lenTotient*sizeof(uint32_t));

        bigNumSub(r1,lenTotient,temp,lenTotient,r1,lenTotient);
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
    free(r1); 
    free(r2);
    free(s2);
    free(temp);
    free(quotient);
    free(bigTemp);
}

uchar *decryptRSA(bignum encryptedMessage,int lenEM,RSAKeyPair kp){
    int sizeDiff = sizeof(uint32_t)/sizeof(uchar); //yes I know it's 4
    int lenMsg = lenEM*sizeDiff;
    uchar *decryptedMessage = (uchar*) malloc(lenMsg);
    bignum pSub1 = (bignum) calloc(kp.privateKey.lenP,sizeof(uint32_t));
    bignum qSub1 = (bignum) calloc(kp.privateKey.lenQ,sizeof(uint32_t));
    bignum totient = (bignum) calloc(kp.publicKey.lenN,sizeof(uint32_t));
    bignum d = (bignum) calloc(kp.publicKey.lenN,sizeof(uint32_t));
    bignum decryptedNum = (bignum) calloc(kp.publicKey.lenN,sizeof(uint32_t));
    if(!decryptedMessage || !pSub1 || !qSub1 || !totient || !d || !decryptedNum){
        free(decryptedMessage);
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
    int j = -1; //increments on first iteration
    for(int i=0;i<lenMsg;i++){
        int mod = i%sizeDiff;
        if(mod==0) j++;
        decryptedMessage[i]  =  (decryptedNum[j] >> (sizeDiff-1-mod)) & 0xff;
    }
    free(pSub1); free(qSub1);
    free(totient); free(d);
    free(decryptedNum);
    return decryptedMessage;
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
    int count = 0;
    do{
        printf("p: %d\n",count++);
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
    printBigNum("p: ",kp.privateKey.p,privateKeyLen);
    printBigNum("q: ",kp.privateKey.q,privateKeyLen);
    printBigNum("n: ",kp.publicKey.n,publicKeyLen);
    kp.publicKey.e = 65537; //Good number for RSA; 65537 == 2**16+1
    return kp;
}


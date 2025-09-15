//RSASSA-PKCS1-v1_5 (SHA 256)
//The official RFC (3447) doesn't specify anything specific
//It is just RSA 

//Used for the CA to sign the certificate
//And the server to sign the handshake
//The private key is hard coded into the server program
//The public key is displayed on the certificate

//Using 2048 bit keys (n is 2048)
//Which is an array of 64 unsigned longs
//Therefore p and q will be 1028 bits(32 length array)
typedef struct PublicKey{
    bignum n;
    unsigned long e;
} PublicKey;

typedef struct PrivateKey{
    bignum p;
    bignum q;
} PrivateKey;


typedef struct KeyPair{
    PrivateKey privateKey;
    PublicKey publicKey;
} KeyPair;


bool isPrime(bignum n,int lenN);
bignum montLadExp(bignum a,int lenA,bignum exp, int lenExp, bignum mod, int modLen);
bool millerRabin(bignum n,int lenN,bignum a,int lenA);
bignum encyrpt(uchar *msg,int lenMsg,unsigned long e,bignum n,int lenN);
bignum extendedEuclidean(unsigned long exp,bignum totient,int lenTotient);
uchar *decrypt(bignum encryptedMessage,int lenEM,bignum d,int lenD,bignum n,int lenN);
KeyPair generateKeys();

bool isPrime(bignum n,int lenN){ 
    for(int i=0;i<30;i++){ //as MR is incorrect 1/4 of time, it is now incorrect 1 in 4^30 times (once every 36558901 years if it runs every ms)
        bignum a = calloc(lenN,sizeof(unsigned long));
        if(!a){
            callocError();
        }
        randomNumber(a,lenN,NULL); //Fermats little theorem only works for 1<a<n-1
        if(!millerRabin(n,lenN,a,lenN)){
            return false;
        }
        free(a);
    }
    //If n passed all the tests, very likely prime
    return true;
}

bignum montLadExp(bignum a,int lenA,bignum exp, int lenExp, bignum mod, int modLen){
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
    int lenLongest = max(lenA,modLen);
    bignum x1 = calloc(lenLongest,sizeof(unsigned long));
    if(!x1){
        callocError();
    }
    memcpy(&x1[lenLongest-lenA],a,lenA*sizeof(unsigned long));
    bignum x2 = bigNumModMult(a,lenA,a,lenA,mod,modLen);
    bool started = false; //start when we reach the first 1 bit (as we have pre-set the first multiplication)
    for(int i=0;i<lenExp;i++){ //MSB to LSB chunks
        for(int j=0;j<32;j++){ //MSB to LSB inside of each chunk
            uint8 bit = (exp[i] >> (31-j))&1;
            if(started){
                if(bit){
                    bigNumModMultRe(x1,lenLongest,x2,modLen,mod,modLen);
                    bigNumModMultRe(x2,modLen,x2,modLen,mod,modLen);
                }
                else{
                    bigNumModMultRe(x2,modLen,x1,lenLongest,mod,modLen);
                    bigNumModMultRe(x1,lenLongest,x1,lenLongest,mod,modLen);
                }
            }
            else if(bit){
                started = true;
            }
            
        }
    }
    free(x2);
    if(lenLongest!=modLen){
        //we know that the MSBs will be empty as it is less than n
        int diff = lenLongest-modLen;
        for(int i=diff;i<lenLongest;i++){
            x1[i-diff] = x1[i];
        }
        x1 = realloc(x1,modLen*sizeof(unsigned long));
    }
    return x1;
}

//Single test case
bool millerRabin(bignum n,int lenN,bignum a,int lenA){
    bignum nSub1= bigNumSubLittle(n,lenN,1,lenN);
    bignum exp = (bignum) calloc(lenN,sizeof(unsigned long));
    if(!nSub1){
        callocError();
    }
    memcpy(exp,nSub1,lenN*sizeof(unsigned long));
    while(!(exp[lenN-1] & 1)){
        bigNumRShiftRe(exp,lenN,1); //Keep shifting until it's odd
    }
    
    //Base case check as last factor is (x-1) whereas all other factors are (x+1)
    bignum factor = montLadExp(a,lenA,exp,lenN,n,lenN);
    //check if x = 1 mod n
    for(int i=0;i<lenN;i++){
        //if a^(n-1/2^k) == 1 mod n  -> if a^(n-1/2^k) -1 is a multiple of n, then n is likely prime 
        if(i==lenN-1 && factor[i]==1){
            free(nSub1);
            free(exp);
            free(factor);
            return true;
        }
        if(factor[i]==0) continue;
        else break;
    }
    while(bignumCmp(exp,nSub1)==LESS_THAN){
        free(factor);
        factor = montLadExp(a,lenA,exp,lenN,n,lenN);
         // a^(n-1/2^k) + 1 is a multiple of n -> satisfying Fermat's Little Theorem -> likely prime
        if(bigNumCmp(factor,lenN,nSub1,lenN)==EQUAL){
            free(nSub1);
            free(exp);
            free(factor);
            return true;
        }
        bigNumLShiftRe(exp,lenN,1); //Binary shift to the left 1 place - same as multiplying by 2
    }
    free(nSub1);
    free(exp);
    free(factor);
    return false; //If no factors are a multiple of n
}


bignum encyrpt(uchar *msg,int lenMsg,unsigned long e,bignum n,int lenN){
    int sizeDiff = sizeof(unsigned long)/sizeof(uchar); //yes I know it's 4
    int lenMsgNum = ceil((float)lenMsg*1/sizeDiff);
    bignum msgNum = calloc(lenMsgNum,sizeof(unsigned long));
    if(!msgNum){
        callocError();
    }
    int j = -1; //increments on first iteration
    for(int i=0;i<lenMsg;i++){
        int mod = i%sizeDiff;
        if(mod==0) j++;
        msgNum[j] |= (unsigned long) msg[i] << ((sizeDiff-1-mod)*8);
    }
    //RSA maths requirement
    if(bigNumCmp(msgNum,lenMsgNum,n,lenN) == GREATER_THAN){
        free(msgNum);
        perror("\nmsg is too long for RSA encryption with this N");
        exit(1);
    }
    unsigned long eBigNum[1] = {e};
    bignum result = montLadExp(msgNum,lenMsgNum,e,1,n,lenN);
    free(msgNum);
    return result;
}

bignum extendedEuclidean(unsigned long exp,bignum totient,int lenTotient){
    bignum r1 = (bignum) calloc(lenTotient,sizeof(unsigned long));
    if(!r1){
        CALLOC_ERROR:
            callocError();
    }
    bignum r2 = (bignum) calloc(lenTotient,sizeof(unsigned long));
    if(!r2){
        FREE_R1:
            free(r1);
            goto CALLOC_ERROR;
    }
    bignum s1 = (bignum) calloc(lenTotient,sizeof(unsigned long));
    if(!s1){
        FREE_R2:
            free(r2);
            goto FREE_R1;
    }
    bignum s2 = (bignum) calloc(lenTotient,sizeof(unsigned long));
    if(!s2){
        FREE_S1:
            free(s1);
            goto FREE_R2;
    }
    bignum temp = (bignum) calloc(lenTotient,sizeof(unsigned long));
    if(!temp){
       free(s2);
       goto FREE_S1;
    }
    
    r1[lenTotient-1] = exp;
    memcpy(r2,totient,lenTotient*sizeof(unsigned long));
    s1[lenTotient-1] = 1;
    //s2 stays at 0
    while(bignumCmp(r2,0) != EQUAL){
        //quotient = r1//r2
        //(r1, r2) = (r2, r1 - quotient *r2) //Just a modulo but quicker as we already have the quotient so this is quicker
        //(s1, s2) = (s2, s1 - quotient *s2) 

        bignum quotient = bigNumDiv(r1,lenTotient,r2,lenTotient);
        memcpy(temp,r2,lenTotient*sizeof(unsigned long));
        
        bignum quotientMultR2 = bigNumMult(quotient,lenTotient,r2,lenTotient,2*lenTotient);
        //We know that quotientMultR2 will fit in lenTotient
        memcpy(r2,quotientMultR2[lenTotient],lenTotient*sizeof(unsigned long));
        quotientMultR2 = realloc(quotientMultR2,lenTotient*sizeof(unsigned long));
        memcpy(quotientMultR2,r2,lenTotient*sizeof(unsigned long));

        bigNumSubRe(r1,lenTotient,quotientMultR2,lenTotient);
        memcpy(r2,r1,lenTotient*sizeof(unsigned long));
        memcpy(r1,temp,lenTotient*sizeof(unsigned long));
        
        //We must stay non-negative
        //The standard euclidean algorithm allows s1 and s2 to be negative but we don't
        bignum quotientMultS2 = bigNumModMult(quotient,lenTotient,s2,lenTotient,totient,lenTotient);
        bignum temp2 = bigNumSub(totient,lenTotient,quotientMultS2,lenTotient,lenTotient);
        bignum temp3 = bigNumModAdd(s1,lenTotient,temp2,lenTotient,totient,lenTotient);
        memcpy(s1,s2,lenTotient*sizeof(unsigned long));
        memcpy(s2,temp3,lenTotient*sizeof(unsigned long));

        free(quotient);
        free(quotientMultR2);
        free(quotientMultS2);
        free(temp2);
        free(temp3);
    }
    free(r1); 
    free(r2);
    free(s2);
    free(temp);
    return s1;
}

uchar *decrypt(bignum encryptedMessage,int lenEM,bignum d,int lenD,bignum n,int lenN){
    int sizeDiff = sizeof(unsigned long)/sizeof(uchar); //yes I know it's 4
    int lenMsg = lenEM*sizeDiff;
    uchar *decryptedMessage = (uchar*) malloc(lenMsg);
    if(!decryptedMessage){
        callocError();
    }
    bignum decryptedNum = montLadExp(encryptedMessage,lenEM,d,lenD,n,lenN);
    int j = -1; //increments on first iteration
    for(int i=0;i<lenMsg;i++){
        int mod = i%sizeDiff;
        if(mod==0) j++;
        decryptedMessage[i]  =  (decryptedNum[j] >> (sizeDiff-1-mod)) & 0xff;
    }
    free(decryptedNum);
    return decryptedMessage;
}

//numBits is the size of the public key n
KeyPair generateKeys(int numBits){
    KeyPair kp;
    int publicKeyLen = numBits/(8*sizeof(unsigned long));
    int privateKeyLen = publicKeyLen/2;
    kp.privateKey.p = calloc(privateKeyLen,sizeof(unsigned long));
    if(!kp.privateKey.p){
        callocError();
    }
    do{
        randomNumber(kp.privateKey.p,privateKeyLen,NULL);
    }
    while(!isPrime(kp.privateKey.p,privateKeyLen));
    kp.privateKey.q = calloc(privateKeyLen,sizeof(unsigned long));
    if(!kp.privateKey.q){
        free(kp.privateKey.p);
        callocError();
    }
    do{
        randomNumber(kp.privateKey.q,privateKeyLen,NULL);
    }
    while(!isPrime(kp.privateKey.q,privateKeyLen));
    kp.publicKey.n = bigNumMult(
        kp.privateKey.p,privateKeyLen,
        kp.privateKey.q,privateKeyLen,
        publicKeyLen
    );
    kp.publicKey.e = 65537; //Good number for RSA; 65537 == 2**16+1

    // totient = (p-1)*(q-1)
    // d = extendedEuclidean(e,totient)
}




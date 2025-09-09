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
    PrivateKey private_key;
    PublicKey public_key;
} KeyPair;


bool isPrime(bignum n,int lenN);
// bool millerRabin(bignum x,int lenX,bignum a,int lenA);
bignum montLadExp(bignum a,int lenA,bignum exp, int lenExp, bignum mod, int modLen);
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
    //Message starts at largest index (big endian)
    int j = lenMsgNum; //decrements on first iteration
    for(int i=lenMsg-1;i>=0;i--){
        int mod = (i-(lenMsg-1))%sizeDiff;
        if(mod==0) j--;
        msgNum[j] |= (unsigned long) msg[i] << (mod*8);
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

// bignum extendedEuclidean(unsigned long exp,bignum totient,int lenTotient){
//     bignum r1 = (bignum) calloc(lenTotient,sizeof(unsigned long));
//     bignum r2 = (bignum) calloc(lenTotient,sizeof(unsigned long));
//     bignum s1 = (bignum) calloc(lenTotient,sizeof(unsigned long));
//     bignum s2 = (bignum) calloc(lenTotient,sizeof(unsigned long));
//     bignum temp = (bignum) calloc(lenTotient,sizeof(unsigned long));
//     (r1, r2) = (exp, tot)
//     (s1, s2) = (1, 0)
//     while r2 != 0:
//         quotient = r1 // r2
//         (r1, r2) = (r2, r1 - quotient *r2) //Just a modulo but quicker as we already have the quotient so this is quicker
//         (s1, s2) = (s2, s1 - quotient *s2) 
//     free(r1); free(r2);
//     free(s1); free(s2);
//     free(temp);
//     return s1 % tot //Non-negativity
// }

// def decrypt(eM, d,n):
//     m = ""   
//     decrypted = binModExp(eM,d,n)
//     binDecrypted = '{0:b}'.format(decrypted)
//     while len(binDecrypted)%16 != 0:
//         binDecrypted = "0"+binDecrypted
//     for i in range(len(binDecrypted)//16):
//         m += chr(int(binDecrypted[i*16:(i+1)*16],2))
//     return m


// minimum = 2**128 //128
// maximum = 2**256 //256
// p = sRandom.randint(minimum,maximum)
// while not isPrime(p):
//     p = sRandom.randint(minimum,maximum)

// q = sRandom.randint(minimum,maximum)
// while not isPrime(q):
//     q = sRandom.randint(minimum,maximum)

// n= p*q 
// e = 65537 //Good number for RSA 65537 = 2**16 +1
// message = input("Input your message ")
// encrypted = encyrpt(message,e,n)
// print("encrypted ",encrypted)


// totient = (p-1)*(q-1)
// d = extendedEuclidean(e,totient)
// message = decrypt(encrypted,d,n)
// print(message)
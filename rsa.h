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
    unsigned long *n;
    unsigned long e;
} PublicKey;

typedef struct PrivateKey{
    unsigned long *p;
    unsigned long *q;
} PrivateKey;


typedef struct KeyPair{
    PrivateKey private_key;
    PublicKey public_key;
} KeyPair;


bool isPrime(unsigned long *x,int lenX);
// bool millerRabin(unsigned long *x,int lenX,unsigned long *a,int lenA);
unsigned long *montLadExp(unsigned long *a,int lenA,unsigned long *exp, int lenExp, unsigned long *mod, int modLen);
KeyPair generateKeys();

bool isPrime(unsigned long *x,int lenX){ 
    for(int i=0;i<30;i++){ //as MR is incorrect 1/4 of time, it is now incorrect 1 in 4^30 times (once every 36558901 years if it runs every ms)
        unsigned long *a = calloc(lenX,sizeof(unsigned long));
        if(!a){
            char errorMsg[20+sizeof(__func__)];
            sprintf(errorMsg,"\nCalloc error in \"%s\"",__func__);
            perror(errorMsg);
            exit(1);
        }
        randomNumber(a,lenX,NULL); //Fermats little theorem only works for 1<a<n-1
        // if(!millerRabin(x,lenX,a,lenX)){
        //     return false;
        // }
        free(a);
    }
    return true;
}

unsigned long *montLadExp(unsigned long *a,int lenA,unsigned long *exp, int lenExp, unsigned long *mod, int modLen){
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
    unsigned long *x1 = calloc(lenA,sizeof(unsigned long));
    if(!x1){
        callocError(__func__);
    }
    memcpy(x1,a,lenA);
    unsigned long *x2 = bigNumModMult(a,lenA,a,lenA,mod,modLen);
    bool started = false; //start when we reach the first 1 bit (as we have pre-set the first multiplication)
    for(int i=0;i<lenExp;i++){ //MSB to LSB chunks
        for(int j=0;j<32;j++){ //MSB to LSB inside of each chunk
            uint8 bit = (exp[i] >> (31-j))&1;
            if(started){
                if(bit){
                    bigNumModMultRe(x2,lenA,x1,lenA,mod,modLen);
                    bigNumModMultRe(x1,lenA,x1,lenA,mod,modLen);
                }
                else{
                    bigNumModMultRe(x1,lenA,x2,lenA,mod,modLen);
                    bigNumModMultRe(x2,lenA,x2,lenA,mod,modLen);
                }
            }
            else if(bit){
                started = true;
            }
            
        }
    }
    free(x2);
    return x1;
}

// bool millerRabin(unsigned long *x,int lenX,unsigned long *a,int lenA){
//     unsigned long *exp = bigNumSubLittle(x,lenX,1,lenX);
//     while(!(exp[lenX-1] & 1)){
//         bigNumRShiftRe(exp,lenX,1); //Keep shifting until it's odd
//     }
    
//     //Base case check as last factor is (x-1) whereas all other factors are (x+1)
//     if montLadExp(a,exp,n) == 1: //if a^(n-1/2^k) == 1 mod n  -> if a^(n-1/2^k) -1 is a multiple of n, then n is likely prime - using pow is more time efficient
//         return True
//     while exp < n-1:
//         if pow(a,exp,n) == n-1: // if a^(n-1/2^k) + 1 is a multiple of n, then n is likely prime 
//             return True
//         exp <<= 1 //Binary shift to the left 1 place - same as multiplying by 2

//     return False //If no factors are a multiple of n
// }

// def binModExp(x,y,z): //x^y mod z
//     r = 1
//     while y!=0:
//         if y & 1: //If last digit is a 1
//             r = r * x % z
//         x = x*x % z
//         y >>=1
//     return r

// def encyrpt(m, e,n):
//     binMessage = ""
//     for i in range(len(m)):
//         unicodePoint = '{0:b}'.format(ord(m[i]))
//         while len(unicodePoint) != 16: //Pad out letter if needed so all letters are uniform
//             unicodePoint = "0"+unicodePoint
//         binMessage += unicodePoint
//     binMessage = int(binMessage,2)
//     if len(str(binMessage)) >= len((str(n))):
//         print("MESSAGE IS TOO LONG")
//         return 0
//     return binModExp(binMessage,e,n)

// def extendedEuclidean(exp,tot):   
//     (r1, r2) = (exp, tot)
//     (s1, s2) = (1, 0)
//     while r2 != 0:
//         quotient = r1 // r2
//         (r1, r2) = (r2, r1 - quotient *r2) //Just a modulo but quicker as we already have the quotient so this is quicker
//         (s1, s2) = (s2, s1 - quotient *s2) 
//     return s1 % tot //Non-negativity

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
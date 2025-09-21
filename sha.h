
//SHA-256 constants 
//Come from first 32 bits of fractional parts of the cube roots of the first 64 prime numbers
unsigned long k256[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

//Initial hash state
//Come from the first 32 bits of the fractional parts of the square roots of the first 8 prime number
unsigned long h0[8] = {
    0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19 
};


typedef struct PaddedMsg{
    uchar *data;
    unsigned long long lenData;
} PaddedMsg;

//lenMsg must be less than 2^61 bytes long - i.e. less than 2*10^6 terabytes
bignum sha256(uchar *msg,unsigned long long lenMsg){
    PaddedMsg paddedMsg = sha256Pad(msg,lenMsg)

    
    bignum hash = calloc(8,sizeof(unsigned long));
    //Set initial hash state
    memcpy(hash,h0,8*sizeof(unsigned long));

    bignum schedule = calloc(64,sizeof(unsigned long));
    for(int )
    free(paddedMsg.data);
}

//Caller must free
PaddedMsg sha256Pad(uchar *msg,unsigned long long lenMsg){
    PaddedMsg result = 
    unsigned long long l = lenMsg*8;
    //l+1+k = 448 mod 512
    //k = 448-(l+1) mod 512
    int lenPadded = 64*ceil(l/512);
    if((l+1)%512>448){
        //We can't fit in l and so we need an extra 512 bit chunk
        lenPadded += 64;
    }
    uchar *paddedMsg = calloc(lenPadded,1);
    if(!paddedMsg){
        allocError();
    }
    //We work out k and set k 0 bits but calloc has already done this
    
    memcpy(paddedMsg,msg,lenMsg);
    //We need to set a 1 bit at the end of the message
    paddedMsg[lenMsg] = 1 << 7;
    //copy the 64 bit representation of l into the end of the message
    for(int i=0;i<8;i++){
        paddedMsg[lenPadded-i] = (l >> i*8) & 0xff;
    }

    return paddedMsg;
}
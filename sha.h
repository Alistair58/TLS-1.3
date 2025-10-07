
//SHA-256 constants 
//Come from first 32 bits of fractional parts of the cube roots of the first 64 prime numbers
uint32_t k256[64] = {
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
uint32_t h0[8] = {
    0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19 
};


typedef struct PaddedMsg{
    uchar *data;
    uint64_t lenData;
} PaddedMsg;

static PaddedMsg sha256Pad(uchar *msg,uint64_t lenMsg);
static void prepareMessageSchedule(bignum msgSchedule,PaddedMsg paddedMsg,int roundNum);
static inline uint32_t sigma_0(uint32_t x);
static inline uint32_t sigma_1(uint32_t x);
static inline uint32_t Sigma_0(uint32_t x);
static inline uint32_t Sigma_1(uint32_t x);
static inline uint32_t Ch(uint32_t x,uint32_t y,uint32_t z);
static inline uint32_t Maj(uint32_t x,uint32_t y,uint32_t z);

//Note: the SHA-256 standard uses big endian and so does my code

//TODO
//Add ROTR, SHR
//Finish main function (add concatentation at the end)
//Test

//lenMsg must be less than 2^61 bytes long - i.e. less than 2*10^6 terabytes
bignum sha256(uchar *msg,uint64_t lenMsg){
    PaddedMsg paddedMsg = sha256Pad(msg,lenMsg);
    int numBlocks = paddedMsg.lenData/64;
    bignum hash = calloc(8,sizeof(uint32_t));
    if(!hash){
        allocError();
    }
    bignum msgSchedule = calloc(64,sizeof(uint32_t));
    if(!msgSchedule){
        free(hash);
        allocError();
    }
    //Set initial hash state
    memcpy(hash,h0,8*sizeof(uint32_t));
    for(int i=0;i<numBlocks;i++){
        prepareMessageSchedule(msgSchedule,paddedMsg,i);
        updateHash(hash,msgSchedule);
        free(msgSchedule);
    }
    free(paddedMsg.data);

    //TODO concat hash words

    return hash;
}

//Caller must free
static PaddedMsg sha256Pad(uchar *msg,uint64_t lenMsg){
    PaddedMsg result;
    uint64_t l = lenMsg*8;
    //l+1+k = 448 mod 512
    //k = 448-(l+1) mod 512
    int lenPadded = 64*ceil(l/512);
    if((l+1)%512>448){
        //We can't fit in l and so we need an extra 512 bit chunk
        lenPadded += 64;
    }
    result.data = calloc(lenPadded,sizeof(uchar));
    result.lenData = lenPadded;
    if(!result.data ){
        allocError();
    }
    //We work out k and set k 0 bits but calloc has already done this
    
    memcpy(result.data,msg,lenMsg);
    //We need to set a 1 bit at the end of the message
    //lenMsg is guaranteed to be in bounds
    result.data[lenMsg] = 1 << 7;
    //Copy the 64 bit representation of l into the end of the message
    for(int i=0;i<8;i++){
        //Fill it from the LSB to MSB in big endian format 
        result.data[lenPadded-1-i] = (l >> i*8) & 0xff;
    }
    return result;
}

static void prepareMessageSchedule(bignum msgSchedule,PaddedMsg paddedMsg,int roundNum){
    memcpy(msgSchedule,&paddedMsg.data[roundNum*64],64);
    for(int t=16;t<64;t++){
        msgSchedule[t] = sigma_1(msgSchedule[t-2]) + msgSchedule[t-7] + 
                        sigma_0(msgSchedule[t-15]) + msgSchedule[t-16];
    }
}

void updateHash(bignum hash,bignum msgSchedule){
    //Variable names from FIPS 180-2
    register uint32_t a = hash[0];
    register uint32_t b = hash[1];
    register uint32_t c = hash[2];
    register uint32_t d = hash[3];
    register uint32_t e = hash[4];
    register uint32_t f = hash[5];
    register uint32_t g = hash[6];
    register uint32_t h = hash[7];
    uint32_t T_1;
    uint32_t T_2;
    for(int t=0;t<64;t++){
        T_1 = h + Sigma_1(e) + Ch(e,f,g) + k256[t] + msgSchedule[t];
        T_2 = Sigma_0(a) + Maj(a,b,c);
        h = g;
        g = f;
        f = e;
        e = d + T_1;
        d = c;
        c = b;
        b = a;
        a = T_1 + T_2;
    }
    hash[0] = a + hash[0];
    hash[1] = b + hash[1];
    hash[2] = c + hash[2];
    hash[3] = d + hash[3];
    hash[4] = e + hash[4];
    hash[5] = f + hash[5];
    hash[6] = g + hash[6];
    hash[7] = h + hash[7];
}

static inline uint32_t sigma_0(uint32_t x){
    return ROTR(x,17) ^ ROTR(x,19) ^ SHR(x,10);
}

static inline uint32_t sigma_1(uint32_t x){
    return ROTR(x,7) ^ ROTR(x,18) ^ SHR(x,3);
}

static inline uint32_t Sigma_0(uint32_t x){
    return ROTR(x,2) ^ ROTR(x,13) ^ ROTR(x,22);
}

static inline uint32_t Sigma_1(uint32_t x){
    return ROTR(x,6) ^ ROTR(x,11) ^ ROTR(x,25);
}

static inline uint32_t Ch(uint32_t x,uint32_t y,uint32_t z){
    return (x & y) ^ (x & z);
}

static inline uint32_t Maj(uint32_t x,uint32_t y,uint32_t z){
    return (x & y) ^ (x & z) ^ (y & z);
}
#include "bigmaths.h"
#include <winerror.h>
#include <stdbool.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "globals.h"

//TODO remove - for profiling
#include <windows.h>
//#define max(a,b) (a)>=(b) ? (a) : (b)

void printBigNum(char *text, bignum n, int lenN){
    printf("\n%s",text);
    for(int i=0;i<lenN;i++){
        printf(" %lu",n[i]);
    }
}


bignum createBigNum(bignum a, int len){ //Caller frees
    bignum p = calloc(len,sizeof(uint32_t));
    if(!p){
        allocError();
    }
    for(int i=0;i<len;i++){
        p[i] = a[i];
    }
    return p;
}



void bigNumAdd(bignum a,int lenA,bignum b, int lenB,bignum dest,int lenDest){ 
    uint64_t temp;
    int carry=0;
    if(lenDest < lenA || lenDest <lenB){
        perror("\nStorage destination for addition is too small");
        exit(1);
    }
    int iA;
    int iB;
    for(int i= lenDest-1;i>-1;i--){
        iA = lenA - (lenDest-i);
        iB = lenB - (lenDest-i);
        uint32_t op1;
        uint32_t op2;
        if(iA<0) op1 = 0;
        else op1 = a[iA];
        if(iB<0) op2 = 0;
        else op2 = b[iB];
        temp = (uint64_t) op1+op2+carry;
        carry = temp >> (sizeof(uint32_t)*8);
        if(carry>0){
            temp &= 0xffffffff;
            if(i==0){
                perror("\nAddition overflow");
                exit(1);
            }
        }    
        dest[i] = temp;
    }
}

void bigNumAddLittle(bignum a,int lenA, uint32_t b,bignum dest,int lenDest){
    uint64_t temp;
    int carry=0;
    if(lenDest < lenA){
        perror("\nStorage destination for addition is too small");
        exit(1);
    }
    int iA;
    for(int i= lenDest-1;i>-1;i--){
        iA = lenA - (lenDest-i);
        uint32_t op1 = (iA<0)?0:a[iA];
        uint32_t op2 = (i==lenDest-1)?b:0;
        temp = (uint64_t) op1+op2+carry;
        carry = temp >> (sizeof(uint32_t)*8);
        if(carry>0){
            temp &= 0xffffffff;
            if(i==0){
                perror("\nAddition overflow");
                exit(1);
            }
        }    
        dest[i] = temp;
    }
}


void bigNumBitModAdd(bignum a,int lenA,bignum b,int lenB,bignum dest,int lenDest,int bitMod, int carryMult){
    bignum sumResult = (bignum) calloc(lenDest+1,sizeof(uint32_t)); //To allow the sum to be bigger than the modded answer
    if(!sumResult){
        allocError();
    }
    bigNumAdd(a,lenA,b,lenB,sumResult,lenDest+1);
    bigNumBitMod(sumResult,lenDest+1,bitMod,carryMult,dest,lenDest);
    free(sumResult);
}


void bigNumMultiAdd(bignum a,int lenA,bignum b,int lenB,bignum c,int lenC,bignum dest,int lenDest){
    bigNumAdd(a,lenA,b,lenB,dest,lenDest);
    bigNumAdd(dest,lenDest,c,lenC,dest,lenDest);
}


//TODO - Currently not actually any more efficient than long multiplication - see book to work out what to reuse
void bigNumMult(bignum a,int lenA, bignum b,int lenB,bignum dest,int lenDest){ 
    /*
    a = [w,x]
    b = [y,z]
    a*b = [wy,wz+xy,xz]
    */
    int longest = max(lenA,lenB);
    if(lenDest<lenA+lenB){
        perror("\nStorage destination for multiplication is too small");
        exit(1);
    }
    if(lenA>1 || lenB>1){
        bool aPadAllocated = false;
        bool bPadAllocated = false;
        bignum aPad = a;
        bignum bPad = b;
        int lenAPad = lenA;
        int lenBPad = lenB;
        if(longest & 1){ //We want the longest to be of even length
            if(longest==lenA){
                aPad = calloc(lenA+1,sizeof(uint32_t));
                aPadAllocated = true;
                memcpy(&aPad[1],a,lenA*sizeof(uint32_t));
                lenAPad++;
            }
            else{
                bPad = calloc(lenB+1,sizeof(uint32_t));
                bPadAllocated = true;
                memcpy(&bPad[1],b,lenB*sizeof(uint32_t));
                lenBPad++;
            }
            longest++;
        }
        if(lenAPad != lenBPad){ //Pad the start with zeroes
            if(longest == lenAPad){
                bPad = calloc(lenAPad,sizeof(uint32_t));
                bPadAllocated = true;
                memcpy(&bPad[lenA-lenB],b,lenB*sizeof(uint32_t));
                for(int i=0;i<(lenA-lenB);i++){
                    bPad[i] = 0;
                }
                lenBPad = longest;
            }
            else{
                aPad = calloc(lenBPad,sizeof(uint32_t));
                aPadAllocated = true;
                memcpy(&aPad[lenB-lenA],a,lenA*sizeof(uint32_t));
                for(int i=0;i<(lenB-lenA);i++){
                    aPad[i] = 0;
                }
                lenAPad = longest;
            }
        }

        const int longestDiv2 = longest/2;

      
        

        //w = aPad[0:longestDiv2) 
        //x = aPad[longestDiv2:longest)
        //y = bPad[0:longestDiv2)
        //z = bPad[longestDiv2:longest) 
        
        bignum wy = calloc(longest,sizeof(uint32_t));
        bignum wz = calloc(longest,sizeof(uint32_t));
        bignum xy = calloc(longest,sizeof(uint32_t));
        bignum xz = calloc(longest,sizeof(uint32_t));
        if(!wy || !wz || !xy || !xz){
            free(wy);free(wz);free(xy);free(xz);
            if(aPadAllocated) free(aPad);
            if(bPadAllocated) free(bPad);
            allocError()
            
        }


        bigNumMult(aPad,longestDiv2,bPad,longestDiv2,wy,longest);
        bigNumMult(aPad,longestDiv2,&bPad[longestDiv2],longestDiv2,wz,longest);
        bigNumMult(&aPad[longestDiv2],longestDiv2,bPad,longestDiv2,xy,longest);
        bigNumMult(&aPad[longestDiv2],longestDiv2,&bPad[longestDiv2],longestDiv2,xz,longest); 
        
        //We don't use aPad or bPad again and hence can use them as buffers (and their length is >=longestDiv2+1)
        bignum pos1Temp = aPad;
        bignum pos2Temp = bPad;
        multiAdd(&wy[longestDiv2],longestDiv2,xy,longestDiv2,wz,longestDiv2,pos1Temp,(longestDiv2)+1); //Accounts for overflow
        multiAdd(&xy[longestDiv2],longestDiv2,&wz[longestDiv2],longestDiv2,xz,longestDiv2,pos2Temp,(longestDiv2)+1);

        bignum pos0 = wz; //reuse the allocated buffers
        bignum pos1 = &wz[longestDiv2];
        bignum pos2 = xy;

        bigNumAdd(wy,longestDiv2,pos1Temp,1,pos0,longestDiv2); //Shouldn't overflow?
        bigNumAdd(&pos1Temp[1],longestDiv2,pos2Temp,1,pos1,longestDiv2);
        memcpy(pos2,&pos2Temp[1],longestDiv2*sizeof(uint32_t));

        int niceLength = (longest<<1);
        int quartered = niceLength>>2;
        int i = max(0,lenDest-niceLength);
        int diff = niceLength-lenDest;
        int j = diff%quartered;
        if(diff <quartered){ 
            memcpy(&dest[i],&pos0[j],(quartered-j)* sizeof(uint32_t));
            i+= quartered - j;
            j=0;
        }
        if(diff < 2*quartered){
            memcpy(&dest[i],&pos1[j],(quartered-j)* sizeof(uint32_t));
            i+=quartered -j;
            j=0;
        }
        if(diff < 3*quartered){
            memcpy(&dest[i],&pos2[j],(quartered-j) * sizeof(uint32_t));
            i+=quartered-j;
            j=0;
        }
        memcpy(&dest[i],&xz[longestDiv2+j],(quartered-j) * sizeof(uint32_t));

        free(wy);free(wz);free(xy);free(xz);
        if(aPadAllocated) free(aPad);
        if(bPadAllocated) free(bPad);
    }
    else{
        uint64_t result = (uint64_t)a[0]*b[0];
        //printf("\nMult result llu %llu ",result);
        uint32_t smallResult = (uint32_t) (result & 0xffffffffUL);
       // printf("\nSmall result %lu ",smallResult);
        uint32_t bigResult = (uint32_t) (result >> 32);
        //printf("\nBig result %lu ",bigResult);
        dest[0] = bigResult;
        dest[1] = smallResult;
        ////printBigNum("Product: ",product,2);
    }
    
}

void bigNumBitModMult(bignum a,int lenA, bignum b,int lenB,bignum dest,int lenDest,int bitMod, int carryMult){
    bignum multResult = (bignum) calloc(lenDest*2,sizeof(uint32_t));
    bigNumMult(a,lenA,b,lenB,multResult,lenDest*2);
    bigNumBitMod(multResult,lenDest*2,bitMod,carryMult,dest,lenDest);
    free(multResult);
}
void bigNumModMult(bignum a,int lenA, bignum b,int lenB,bignum n, int lenN,bignum dest,int lenDest){ 
    //will return a big num of size lenN
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);

    LARGE_INTEGER start, end;
    QueryPerformanceCounter(&start);

    bignum multResult = (bignum) calloc(lenA+lenB,sizeof(uint32_t));
    bigNumMult(a,lenA,b,lenB,multResult,lenA+lenB);

    QueryPerformanceCounter(&end);
    double multElapsedUs = (double)(end.QuadPart - start.QuadPart) * 1e6 / (double)freq.QuadPart;
    printf("\nMult us: %.3f",multElapsedUs);
    QueryPerformanceCounter(&start);
    bigNumMod(multResult,lenA+lenB,n,lenN,dest,lenDest);
    QueryPerformanceCounter(&end);
    double modElapsedUs = (double)(end.QuadPart - start.QuadPart) * 1e6 / (double)freq.QuadPart;
    printf("\nMod us: %.3f",modElapsedUs);

    free(multResult);
}


void bigNumBitMod(bignum a, int lenA,int bitMod,int carryMult,bignum dest,int lenDest){
    int i = (int)((float)lenA - (float)bitMod/32); //Do we need to mod the number, if so how many chunks do we need to mod
    if(!(bitMod%32)) i--; //Doesn't affect 25519
    if(i<0){
        return;
    }
    
    int lenRealCarry = (!(bitMod%32))?2:1; //Doesn't work with addition and ternary operator
    lenRealCarry += i;
    bignum realCarry = calloc(lenRealCarry,sizeof(uint32_t));
    bignum carry = calloc(i+1,sizeof(uint32_t)); //Allocate enough chunks
    if(!carry || !realCarry){
        free(carry);
        free(realCarry);
        perror("\nCalloc error during mod");
        exit(1);
    }
    do{//Repeat until no more modding is required, 1 iteration on 0xfffff will require another mod
        uint32_t doubleCarry = 0;
        int bitDepth = bitMod%32; //How far to go into the first chunk
        
        for(int j=0;j<=i;j++){ //Go through all the chunks that are too big
            if(j==i){
                carry[j] = (a[j] >> bitDepth) + doubleCarry;
                a[j] -= (a[j] >> bitDepth) << bitDepth; //get rid of the big bits
            }
            else{
                //Double carry ensures that the positioning of array elements is correct
                //a = [j-1]{...bitDepth abc},[j]{xyz BitDepth def}, 
                //carry[j] = abcxyz
                //doubleCarry = def 
                carry[j] = (a[j] >> bitDepth) + doubleCarry; //e.g. if you are doing bitDepth 31 and there is a {...2},{1} carry this should be represented as {0,2*2^1 + 1}
                doubleCarry = (a[j] - ((a[j] >> bitDepth)<<bitDepth)) * (1ul<<(32-bitDepth)); 
                a[j] = 0;
            }
        }
        
        bigNumMultByLittle(carry,i+1,carryMult,realCarry,lenRealCarry);

        bigNumAdd(realCarry,lenRealCarry,a,lenA,a,lenA);

        if(a[i] < (1<<bitDepth) && (i==0 || a[i-1] == 0)) break;
        
        
    }
    while(true);

    free(carry);free(realCarry);

    memcpy(dest,&a[lenA-lenDest],lenDest*sizeof(uint32_t));
}



void bigNumMultByLittle(bignum a,int lenA, uint32_t littleNum,bignum dest,int lenDest){
    if(lenDest<lenA){
        perror("\nStorage destination for multiplication is too small");
        exit(1);
    }
    bignum product = calloc(lenDest,sizeof(uint32_t));
    if(!product){
        allocError();
    }
    uint32_t carry = 0;
    for(int i=lenA-1;i>-1;i--){ 
        int pI = lenDest - (lenA-i);
        uint64_t result = (uint64_t)a[i]*littleNum + carry;
        uint32_t thisChunk = result & 0xffffffff;
        uint32_t carry = result >> 32;  
        product[pI] = thisChunk;
        if(i==0 && carry){
            if(lenDest>lenA){
                product[pI-1] = carry;
            }
            else{
                free(product);
                perror("\nMultiplication overflow");
                exit(1);
            }
            
        }
    }
    return product;
    
}

void bigNumBitModMultByLittle(bignum a,int lenA, uint32_t littleNum,bignum dest,int lenDest,int bitMod, int carryMult){
    bignum multResult = (bignum) calloc(lenDest+1,sizeof(uint32_t));
    bigNumMultByLittle(a,lenA,littleNum,multResult,lenDest+1);
    bigNumBitMod(multResult,lenDest+1,bitMod,carryMult,dest,lenDest);
    free(multResult);
}

void bigNumSub(bignum a,int lenA, bignum b,int lenB,bignum dest,int lenDest){
    long long temp;
    int carry=0;
    if(bigNumCmp(a,lenA,b,lenB)==LESS_THAN){
        perror("\nFirst argument must be larger than second for subtraction");
        exit(1);
    }
    if(lenDest<lenA){
        perror("\nlenDest must be greater than or equal to lenA for subtraction");
        exit(1);
    }
    int iA;
    int iB;
    for(int i= lenDest-1;i>-1;i--){
        iA = lenA - (lenDest-i);
        iB = lenB - (lenDest-i); 
        uint32_t op1,op2;
        if(iA<0) op1 = 0;
        else op1 = a[iA];
        if(iB<0) op2 = 0;
        else op2 = b[iB];
        temp = (long long) op1-op2+carry;
        carry = temp >> (sizeof(uint32_t)*8); //check for negative number
        if(carry){
            temp &= 0xffffffff;
            if(i==0){
                perror("\nFirst argument must be larger than second for subtraction");
                exit(1);
            }
        }    
        dest[i] = temp;
    }
}

void bigNumSubLittle(bignum a,int lenA, uint32_t b,bignum dest,int lenDest){
    long long temp;
    int carry=0;
    int iA;
    for(int i= lenDest-1;i>-1;i--){
        iA = lenA - (lenDest-i);
        uint32_t op1 = (iA<0)?0:a[iA];
        uint32_t op2 = (i==lenDest-1)?b:0;
        temp = (long long) op1-op2+carry;
        carry = temp >> (sizeof(uint32_t)*8); //check for negative number
        if(carry){
            temp &= 0xffffffff;
            if(i==0){
                perror("\nFirst argument must be smaller than second for subtraction");
                exit(1);
            }
        }    
        dest[i] = temp;
    }
}

void bigNumModSub(bignum a,int lenA,bignum b,int lenB,bignum dest,int lenDest,bignum p,int lenP){
    long long temp=0,carry=0;
    int iA,iB;
    for(int i= lenDest-1;i>-1;i--){
        iA = lenA - (lenDest-i);
        iB = lenB - (lenDest-i);        
        uint32_t op1;
        uint32_t op2;
        if(iA<0) op1 = 0;
        else op1 = a[iA];
        if(iB<0) op2 = 0;
        else op2 = b[iB];
        
        temp = (long long) op1- (long long) op2 + (long long) carry; //If you don't cast, you don't get any negatives
        carry = (long long) temp >> 32; //If this is negative or any previous digits have been negative
        
        if(carry){
            int iP = lenP - (lenDest-i);
            if(iP<0){
                perror("\nError? - bigNumModSub");
                exit(1);
            }
            if(i==0){
                int j=255,bit=0; //Turn it positive and subtract it from p
                uint32_t notted,chunk;
                while(bit==0){
                    if(j<32){
                        bit = (temp >> (31-(j & 31))) & 1;
                    }
                    else{
                        bit = (dest[j >> 5] >> (31-(j & 31))) & 1;
                    }
                    j--;
                    
                }
                while(j>=0){
                    if(j<32){
                        chunk = (temp >> (31-(j & 31)));
                        notted = ~(chunk << (31-(j & 31)));
                    }
                    else{
                        chunk = (dest[j >> 5] >> (31-(j & 31)));
                        notted = ~(chunk << (31-(j & 31)));
                        notted -= ((1ul<<(31-(j & 31)))-1); //Remove the following bits which have been turned to 1s
                        // 0110110111
                        // 0110110000
                        // 1001001111
                        // 1001000000
                    }
        
                    if(j%32 != 31){
                        dest[j >> 5] = notted + (dest[j>>5]&((uint32_t)(1ul<<(31-(j & 31)))-1));
                        j-= (j&31)+1;
                    }
                    else{
                        dest[j >> 5] = notted; 
                        j-= 32;
                    }
                }
                bigNumSub(p,lenP,dest,lenDest,dest,lenDest); //Will be positive
            }
            else{ //If i!=0
                temp -= (long long) carry << 32; //Make it positive
                dest[i] = temp;
            }
        }
        else{ //No carry
            dest[i] = temp;
        }
    }
}

void bigNumBitModInv(bignum a,int lenA,bignum p,int lenP,bignum dest,int lenDest,int bitMod,int carryMult){
    //inv a = a^p-2
    //Constant time as p-2 is constant (for any given curve)
    if(lenDest!=lenA){
        perror("\nbigNumBitModInv: lenA == lenDest failed");
        exit(1);
    }
    int i = 1,bit;
    bool started = false;
    bignum pCpy = calloc(lenP,sizeof(uint32_t));
    
    bignum temp1,temp2;
    bignum modTemp1,modTemp2;

    if(!dest || !pCpy){
        allocError();
    }
    dest[lenA-1] = 1;
    memcpy(pCpy,p,lenP*sizeof(uint32_t));
    while(i<256){  //MSB to LSB
        bigNumBitModMult(dest,lenA,dest,lenA,dest,lenA,bitMod,carryMult);
        bit = (pCpy[i >> 5] >> (31-(i & 31))) & 1;
        if(bit){
            bigNumBitModMult(dest,lenA,a,lenA,dest,lenDest,bitMod,carryMult);
        }
        i++;
        
    }
}

void bigNumRShift(bignum a,int lenA,int shift,bignum dest,int lenDest){ 
    if(shift>=32){
        perror("\nShift must be less than 32");
        exit(1);
    }
    if(lenDest!=lenA){
        perror("\nbigNumRShift: lenA == lenDest failed");
        exit(1);
    }
    uint32_t carry = 0;
    uint32_t carryBitMask = ((1<<(shift))-1);
    for(int i=0;i<lenA;i++){
        uint32_t temp = a[i] & carryBitMask; //Save the LSBs
        dest[i] = a[i] >> shift; 
        if(carry) dest[i] |= carry << (32-shift); //add in the previous LSBs
        carry = temp;
    }
}

void bigNumLShift(bignum a,int lenA,int shift,bignum dest,int lenDest){
    if(shift>=32){
        perror("\nShift must be less than 32");
        exit(1);
    }
    if(lenDest!=lenA){
        perror("\nbigNumLShift: lenA == lenDest failed");
        exit(1);
    }
    uint32_t carry = 0;
    for(int i=lenA-1;i>=0;i--){
        uint64_t temp = ((uint64_t) a[i]) << shift; 
        dest[i] = ((uint32_t) (temp & ULONG_MAX)) + carry;
        carry = (uint32_t) ((temp+(uint64_t)carry) >> 32);
    }
    if(carry){
        perror("\nOverflow error on bigNumLShift");
        exit(1);
    }
}

uint8 bigNumCmp(bignum a,int lenA,bignum b,int lenB){
    int lenLongest = max(lenA,lenB);
    int iA,iB;
    uint32_t op1,op2;
    for(int i=0;i<lenLongest;i++){
        iA = lenA - (lenLongest-i);
        iB = lenB - (lenLongest-i);
        op1 = (iA<0)?0:a[iA];
        op2 = (iB<0)?0:b[iB];
        if(op1<op2) return LESS_THAN;
        else if(op1>op2) return GREATER_THAN;
    }
    return EQUAL;
}

uint8 bigNumCmpLittle(bignum a,int lenA,uint32_t b){
    for(int i=0;i<(lenA-1);i++){
        if(a[i]!=0) return GREATER_THAN;
    }
    if(a[lenA-1]>b) return GREATER_THAN;
    else if(a[lenA-1]<b) return LESS_THAN;
    else return EQUAL;
}

//Returns size lenN
void bigNumMod(bignum a,int lenA,bignum n,int lenN,bignum dest,int lenDest){ //a % n
    if(lenN != lenDest){
        perror("\nbigNumMod: lenN == lenDest check failed");
        exit(1);
    }
    //Long division
    //remainder must be of size lenN+1 so that it can handle an extra shift before being subtracted
    bignum remainder = (bignum) calloc(lenN+1,sizeof(uint32_t));
    if(!remainder){
        allocError();
    }
    int bits = lenA*32;
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);

    LARGE_INTEGER start, end;
    QueryPerformanceCounter(&start);
    
    for(int i=0;i<bits;i++){
        bigNumLShift(remainder,lenN+1,1,remainder,lenN+1);
        int bit = ((a[i>>5] >> (31 - (i&31))) & 1);
        remainder[lenN] |= bit; //set LSB of curr to be the current bit in a
        uint8 cmpRes = bigNumCmp(remainder,lenN+1,n,lenN);
        if(cmpRes != LESS_THAN){ //If equal or greater
            bigNumSubRe(remainder,lenN+1,n,lenN);
            //Don't need to actually work out the quotient
        }
    }
    QueryPerformanceCounter(&end);
    double modBodyElapsedUs = (double)(end.QuadPart - start.QuadPart) * 1e6 / (double)freq.QuadPart;
    printf("\nMod body us: %.3f",modBodyElapsedUs);
    //Return the left over carry which will be the remainder
    memcpy(dest,&remainder[1],lenN*sizeof(uint32_t));
    free(remainder);
}

//Returns size lenA
void bigNumDiv(bignum a,int lenA,bignum b,int lenB,bignum dest,int lenDest){ //returns the quotient
    //Long division
    if(lenA != lenDest){
        perror("\nbigNumDiv: lenA == lenDest check failed");
        exit(1);
    }
    //Set dest to 0
    for(int i=0;i<lenDest;i++){
        dest[i] = 0;
    }
    bignum remainder = (bignum) calloc(lenB+1,sizeof(uint32_t));
    if(!remainder){
        allocError();
    }
    int bits = lenA*32;
    for(int i=0;i<bits;i++){
        bigNumLShift(remainder,lenB+1,1,remainder,lenB+1);
        int bit = ((a[i>>5] >> (31 - (i&31))) & 1);
        remainder[lenB] |= bit; //set LSB of curr to be the current bit in a
        uint8 cmpRes = bigNumCmp(remainder,lenB+1,b,lenB);
        if(cmpRes != LESS_THAN){ //Equal or greater than
            bigNumSubRe(remainder,lenB+1,b,lenB);
            dest[i>>5] |= (1 << (31 - (i&31)));
            //Don't need to actually work out the quotient
        }
    }
    //Don't need to mess around with remainder 
} 

void bigNumModAdd(bignum a,int lenA, bignum b, int lenB,bignum n,int lenN,bignum dest,int lenDest){
    if(lenDest != lenN){
        perror("\nbigNumModAdd: lenDest == lenN check failed");
        exit(1);
    }
    int lenUnmodded = max(lenA,lenB)+1;
    bignum unmodded = calloc(lenUnmodded,sizeof(uint32_t));
    if(!unmodded){
        allocError();
    }
    bigNumAdd(a,lenA,b,lenB,unmodded,lenUnmodded);
    bigNumMod(unmodded,lenUnmodded,n,lenN,dest,lenDest);
    free(unmodded);
}
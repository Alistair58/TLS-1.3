#include "bigmaths.h"
#include <winerror.h>
#include <stdbool.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "globals.h"

#define max(a,b) (a)>=(b) ? (a) : (b)

void printBigNum(char *text, bignum n, int lenN){
    printf("\n%s",text);
    for(int i=0;i<lenN;i++){
        printf(" %lu",n[i]);
    }
}


bignum createBigNum(bignum a, int len){ //MUST REMEMBER TO FREE IF USING THIS
    bignum p = calloc(len,sizeof(uint32_t));
    if(!p){
        allocError();
    }
    for(int i=0;i<len;i++){
        p[i] = a[i];
    }
    return p;
}



bignum bigNumAdd(bignum a,int lenA, bignum b, int lenB, int lenDest){
    uint64_t temp;
    int carry=0;
    if(lenDest < lenA || lenDest <lenB){
        
        perror("\nStorage destination for addition is too small");
        exit(1);
    }
    bignum sum = calloc(lenDest,sizeof(uint32_t));
    if(!sum){
        allocError();
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
                free(sum);
                perror("\nAddition overflow");
                exit(1);
            }
        }    
        sum[i] = temp;
    }
    return sum;
}

bignum bigNumAddLittle(bignum a,int lenA, uint32_t b, int lenDest){
    uint64_t temp;
    int carry=0;
    if(lenDest < lenA){
        perror("\nStorage destination for addition is too small");
        exit(1);
    }
    bignum sum = calloc(lenDest,sizeof(uint32_t));
    if(!sum){
        allocError();
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
                free(sum);
                perror("\nAddition overflow");
                exit(1);
            }
        }    
        sum[i] = temp;
    }
    return sum;
}


bignum bigNumBitModAdd(bignum a,int lenA, bignum b,int lenB,int lenDest,int bitMod, int carryMult){
    bignum addResult = bigNumAdd(a,lenA,b,lenB,lenDest+1);
    bignum modResult = bigNumBitMod(addResult,lenDest+1,bitMod,carryMult,lenDest);
    free(addResult);
    return modResult;
}


bignum  multiAdd(bignum a,int lenA,bignum b, int lenB, bignum c,int lenC, int lenDest){
    bignum sum1 = bigNumAdd(a,lenA,b,lenB,lenDest);
    bignum sum2 = bigNumAdd(sum1,lenDest,c,lenC,lenDest);
    free(sum1);
    return sum2;
}

void bigNumMultRe(bignum a,int lenA, bignum b,int lenB){ //Multiply a and b and store in a
    bignum res = bigNumMult(a,lenA,b,lenB,lenA);
    a = realloc(a,lenA*sizeof(uint32_t)); //bigNumMult may pad A if necessary
    memcpy(a,res,lenA*sizeof(uint32_t));
    free(res);
}

//TODO - Currently not actually any more efficient than long multiplication - see book to work out what to reuse
bignum bigNumMult(bignum a,int lenA, bignum b,int lenB,int lenDest){ 
    /*a = [a1,a2]
    b = [b1,b2]
    a*b = [a1*b1,a1*b2+a2*b1,a2*b2]
    */
    int longest = max(lenA,lenB);
    if(lenDest<lenA+lenB){
        free(a); free(b);
        perror("\nStorage destination for multiplication is too small");
        exit(1);
    }
    bignum product = calloc(lenDest,sizeof(uint32_t));
    if(!product){
        allocError();
    }
    if(lenA>1 || lenB>1){
        if(longest & 1){
            if(longest==lenA){
                a = realloc(a,(lenA+1)*sizeof(uint32_t));
                memcpy(&a[1],a,lenA*sizeof(uint32_t));
                a[0] = 0;
                lenA++;
            }
            else{
                b = realloc(b,(lenB+1)*sizeof(uint32_t));
                memcpy(&b[1],b,lenB*sizeof(uint32_t));
                b[0] = 0;
                lenB++;
            }
            longest++;
        }
        if(lenA != lenB){ //Pad the start with zeroes
            if(longest == lenA){
                b = realloc(b,lenA*sizeof(uint32_t));
                memcpy(&b[lenA-lenB],b,lenB*sizeof(uint32_t));
                for(int i=0;i<(lenA-lenB);i++){
                    b[i] = 0;
                }
                lenB = longest;
            }
            else{
                a = realloc(a,lenB*sizeof(uint32_t));
                memcpy(&a[lenB-lenA],a,lenA*sizeof(uint32_t));
                for(int i=0;i<(lenB-lenA);i++){
                    a[i] = 0;
                }
                lenA = longest;
            }
        }

        uint32_t w[longest/2]; uint32_t x[longest/2]; uint32_t y[longest/2]; uint32_t z[longest/2];
        uint32_t wy1[longest/2];uint32_t wy2[longest/2]; uint32_t wz1[longest/2];uint32_t wz2[longest/2];
        uint32_t xy1[longest/2];uint32_t xy2[longest/2];uint32_t xz1[longest/2];uint32_t xz2[longest/2];

        memcpy(w, a, longest/2 * sizeof(uint32_t)); memcpy(x, &a[longest/2], longest/2 * sizeof(uint32_t)); 
        memcpy(y, b, longest/2 * sizeof(uint32_t)); memcpy(z, &b[longest/2], longest/2 * sizeof(uint32_t)); 
        
        bignum wy = bigNumMult(w,longest/2,y,longest/2,longest);
        bignum wz = bigNumMult(w,longest/2,z,longest/2,longest);
        bignum xy = bigNumMult(x,longest/2,y,longest/2,longest);
        bignum xz = bigNumMult(x,longest/2,z,longest/2,longest);

        memcpy(wy1, wy, longest/2 * sizeof(uint32_t)); memcpy(wy2, &wy[longest/2], longest/2 * sizeof(uint32_t));  
        memcpy(wz1, wz, longest/2 * sizeof(uint32_t)); memcpy(wz2, &wz[longest/2], longest/2 * sizeof(uint32_t));  
        memcpy(xy1, xy, longest/2 * sizeof(uint32_t)); memcpy(xy2, &xy[longest/2], longest/2 * sizeof(uint32_t));  
        memcpy(xz1, xz, longest/2 * sizeof(uint32_t)); memcpy(xz2, &xz[longest/2], longest/2 * sizeof(uint32_t));  

        
        bignum pos1Temp = multiAdd(wy2,longest/2,xy1,longest/2,wz1,longest/2,(longest/2)+1); //Accounts for overflow
        bignum pos2Temp = multiAdd(xy2,longest/2,wz2,longest/2,xz1,longest/2,(longest/2)+1);
        bignum pos0 = bigNumAdd(wy1,longest/2,pos1Temp,1,longest/2); //Shouldn't overflow?
        bignum pos1 = bigNumAdd(&pos1Temp[1],longest/2,pos2Temp,1,longest/2);
        bignum pos2 = calloc(longest/2,sizeof(uint32_t));
        memcpy(pos2,&pos2Temp[1],longest/2*sizeof(uint32_t));
        free(pos1Temp);free(pos2Temp);
        int niceLength = (longest<<1);
        int quartered = niceLength>>2;
        int i = max(0,lenDest-niceLength);
        int diff = niceLength-lenDest;
        int j = diff%quartered;
        //printf("\ni %d",i);
        if(diff <quartered){ //Used to be <= 
            memcpy(&product[i],&pos0[j],(quartered-j)* sizeof(uint32_t));
            i+= quartered - j;
            //printf("\ni %d",i);
            j=0;
        }
        if(diff < 2*quartered){
            memcpy(&product[i],&pos1[j],(quartered-j)* sizeof(uint32_t));
            i+=quartered -j;
            //printf("\ni %d",i);
            j=0;
        }
        if(diff < 3*quartered){
            memcpy(&product[i],&pos2[j],(quartered-j) * sizeof(uint32_t));
            i+=quartered-j;
            //printf("\ni %d",i);
            j=0;
        }
        //printf("\ni %d j %d",i,j);
        memcpy(&product[i],&xz2[j],(quartered-j) * sizeof(uint32_t));
        ////printBigNum("Mult product: ",product,lenDest);

        free(wy);free(wz);free(xy);free(xz);
        free(pos0);free(pos1);free(pos2);
    }
    else{
        uint64_t result = (uint64_t)a[0]*b[0];
        //printf("\nMult result llu %llu ",result);
        uint32_t smallResult = (uint32_t) (result & 0xffffffffUL);
       // printf("\nSmall result %lu ",smallResult);
        uint32_t bigResult = (uint32_t) (result >> 32);
        //printf("\nBig result %lu ",bigResult);
        product[0] = bigResult;
        product[1] = smallResult;
        ////printBigNum("Product: ",product,2);
    }
    return product;
}

bignum bigNumBitModMult(bignum a,int lenA, bignum b,int lenB,int lenDest,int bitMod, int carryMult){
    bignum multResult = bigNumMult(a,lenA,b,lenB,lenDest*2);
    bignum modResult = bigNumBitMod(multResult,lenDest*2,bitMod,carryMult,lenDest);
    free(multResult);
    return modResult;
}
bignum bigNumModMult(bignum a,int lenA, bignum b,int lenB,bignum n, int lenN){ 
    //will return a big num of size lenN
    bignum multResult = bigNumMult(a,lenA,b,lenB,lenA+lenB);
    bigNumModRe(multResult,lenA+lenB,n,lenN);
    //The mod length could (and will likely) be less than lenA+lenB 
    //e.g. {0,0,0,2} needs to be turned to {0,2} if lenN=2
    int lenDifference = (lenA+lenB)-lenN;
    for(int i=lenDifference;i<(lenA+lenB);i++){
        multResult[i-lenDifference] = multResult[i]; 
    }
    multResult = realloc(multResult,lenN*sizeof(uint32_t));
    return multResult;
}

bignum bigNumModMultRe(bignum a,int lenA, bignum b,int lenB,bignum n, int lenN){ //store it in a 
    if(lenA<lenN){
        perror("\nInsufficient space to store result of \"bigNumModMultRe\" in argument 1");
        exit(ERROR_BAD_LENGTH);
    }
    bignum result = bigNumModMult(a,lenA,b,lenB,n,lenN);
    memcpy(a,result,lenA*sizeof(uint32_t));
    free(result);
}

bignum bigNumBitMod(bignum a, int lenA,int bitMod,int carryMult, int lenDest){
    bignum product = calloc(lenDest,sizeof(uint32_t));
    if(!product){
        allocError();
    }
    int i = (int)((float)lenA - (float)bitMod/32); //Do we need to mod the number, if so how many chunks do we need to mod
    if(!(bitMod%32)) i--; //Doesn't affect 25519
    if(i<0){
        memcpy(product,a,lenA*sizeof(uint32_t));
        return product;
    }
    bignum carry,realCarry,addedCarry;
    uint32_t doubleCarry;
    do{//Repeat until no more modding is require, 1 iteration on 0xfffff will require another mod
        doubleCarry = 0;
        int bitDepth = bitMod%32; //How far to go into the first chunk
        carry = calloc(i+1,sizeof(uint32_t)); //Allocate enough chunks
        if(!carry){
            perror("\nCalloc error during mod");
            exit(1);
        }
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
        int lenRealCarry = (!(bitMod%32))?2:1; //Doesn't work with addition and ternary operator
        lenRealCarry += i;
        realCarry = bigNumMultByLittle(carry,i+1,carryMult,lenRealCarry);

        addedCarry = bigNumAdd(realCarry,lenRealCarry,a,lenA,lenA);
        memcpy(a,addedCarry, lenA * sizeof(uint32_t));

        if(a[i] < (1<<bitDepth) && (i==0 || a[i-1] == 0)) break;
        free(carry);free(realCarry);free(addedCarry);
        
    }
    while(true);
    memcpy(product,&a[lenA-lenDest],lenDest*sizeof(uint32_t));
    return product;
}



bignum bigNumMultByLittle(bignum a,int lenA, uint32_t littleNum,int lenDest){
    if(lenDest<lenA){
        perror("\nStorage destination for multiplication is too small");
        exit(1);
    }
    bignum product = calloc(lenDest,sizeof(uint32_t));
    if(!product){
        allocError();
    }
    uint32_t thisChunk = 0;
    uint32_t carry = 0;
    int pI;
    for(int i=lenA-1;i>-1;i--){ //TODO - doesn't take into account lenDest and so overflows on any possible overflow
        pI = lenDest - (lenA-i);
        uint64_t result = (uint64_t)a[i]*littleNum + carry;
        thisChunk = result & 0xffffffff;
        carry = result >> 32;  
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

bignum bigNumBitModMultByLittle(bignum a,int lenA, uint32_t littleNum,int lenDest,int bitMod, int carryMult){
    bignum multResult = bigNumMultByLittle(a,lenA,littleNum,lenDest*2);
    bignum modResult = bigNumBitMod(multResult,lenDest*2,bitMod,carryMult,lenDest);
    free(multResult);
    return modResult;
}

bignum bigNumSub(bignum a,int lenA, bignum b,int lenB,int lenDest){
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
    bignum result = calloc(lenDest,sizeof(uint32_t));
    if(!result){
        allocError();
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
        temp = (long long) op1-op2+carry;
        carry = temp >> (sizeof(uint32_t)*8); //check for negative number
        if(carry){
            temp &= 0xffffffff;
            if(i==0){
                free(result);
                perror("\nFirst argument must be larger than second for subtraction");
                exit(1);
            }
        }    
        result[i] = temp;
    }
    return result;
}

void bigNumSubRe(bignum a,int lenA,bignum b,int lenB){ //stores it in a
    bignum result = bigNumSub(a,lenA,b,lenB,lenA);
    memcpy(a,result,lenA*sizeof(uint32_t));
    free(result);
}


bignum bigNumSubLittle(bignum a,int lenA, uint32_t b,int lenDest){
    long long temp;
    int carry=0;
    bignum result = calloc(lenDest,sizeof(uint32_t));
    if(!result){
        allocError();
    }
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
                free(result);
                perror("\nFirst argument must be smaller than second for subtraction");
                exit(1);
            }
        }    
        result[i] = temp;
    }
    return result;
}

bignum bigNumModSub(bignum a,int lenA, bignum b,int lenB,int lenDest,bignum p,int lenP){
    //Answer is guaranteed in range (2^bitmod-carryMult,-(2^bitmod-carryMult))
    //As the inputs have already been modded
    // -25 mod 256 = 231 = 256 -25
    //
    long long temp=0,carry=0;
    bignum result = calloc(lenDest,sizeof(uint32_t));
    if(!result){
        allocError();
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
        
        temp = (long long) op1- (long long) op2 + (long long) carry; //If you don't cast, you don't get any negatives
        carry = (long long) temp >> 32; //If this is negative or any previous digits have been negative
        
        if(carry){
            int iP = lenP - (lenDest-i);
            if(iP<0){
                free(result);free(a);free(b);
                perror("Error? - bigNumModSub");
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
                        bit = (result[j >> 5] >> (31-(j & 31))) & 1;
                    }
                    j--;
                    
                }
                while(j>=0){
                    if(j<32){
                        chunk = (temp >> (31-(j & 31)));
                        notted = ~(chunk << (31-(j & 31)));
                    }
                    else{
                        chunk = (result[j >> 5] >> (31-(j & 31)));
                        notted = ~(chunk << (31-(j & 31)));
                        notted -= ((1ul<<(31-(j & 31)))-1); //Remove the following bits which have been turned to 1s
                        // 0110110111
                        // 0110110000
                        // 1001001111
                        // 1001000000
                    }
                    
                    if(j%32 != 31){
                        result[j >> 5] = notted + (result[j>>5]&((uint32_t)(1ul<<(31-(j & 31)))-1));
                        j-= (j&31)+1;
                    }
                    else{
                        result[j >> 5] = notted; 
                        j-= 32;
                    }
                    
                }
                bignum modded = bigNumSub(p,lenP,result,lenDest,lenDest); //Will be positive
                memcpy(result,modded,lenDest*sizeof(uint32_t));
                free(modded);

            }
            else{
                temp -= (long long) carry << 32; //Make it positive
                result[i] = temp;
            }
        }
        else{
            result[i] = temp;
        }
        
        
    }
    return result;
}

bignum bigNumBitModInv(bignum a, int lenA,bignum p, int lenP, int lenDest,int bitMod,int carryMult){
    //inv a = a^p-2
    //Constant time as p-2 is constant (for any given curve)

    //Not tested but follows the paper exactly
    int i = 1,bit;
    bool started = false;
    bignum pCpy = calloc(lenP,sizeof(uint32_t));
    bignum r = calloc(lenA,sizeof(uint32_t));
    
    bignum temp1,temp2;
    bignum modTemp1,modTemp2;

    if(!r || !pCpy){
        allocError();
    }
    r[lenA-1] = 1;
    memcpy(pCpy,p,lenP*sizeof(uint32_t));
    while(i<256){  //MSB to LSB
        temp1 = bigNumMult(r,lenA,r,lenA,lenA*2);
        modTemp1 = bigNumBitMod(temp1,lenA*2,bitMod,carryMult,lenA);
        memcpy(r,modTemp1,lenA * sizeof(uint32_t));
        free(temp1); free(modTemp1);

        bit = (pCpy[i >> 5] >> (31-(i & 31))) & 1;
        if(bit){
            temp2 = bigNumMult(r,lenA,a,lenA,lenA*2);
            modTemp2 = bigNumBitMod(temp2,lenA*2,bitMod,carryMult,lenA);
            memcpy(r,modTemp2,lenA * sizeof(uint32_t));
            free(temp2); free(modTemp2);
        }
        i++;
        
    }
    return r;
}

bignum bigNumRShift(bignum a,int lenA,int shift){ 
    if(shift>=32){
        perror("\nShift must be less than 32");
        exit(1);
    }
    bignum result = calloc(lenA,sizeof(uint32_t));
    if(!result){
        allocError();
    }
    uint32_t carry = 0,temp;
    uint32_t carryBitMask = ((1<<(shift))-1);
    for(int i=0;i<lenA;i++){
        temp = a[i] & carryBitMask; //Save the LSBs
        result[i] = a[i] >> shift; 
        if(carry) result[i] &= carry << (32-shift); //add in the previous LSBs
        carry = temp;
    }
    return result;
}

void bigNumRShiftRe(bignum a,int lenA,int shift){  
    bignum result = bigNumRShift(a,lenA,shift);
    memcpy(a,result,lenA*sizeof(uint32_t));
    free(result);
}

bignum bigNumLShift(bignum a,int lenA,int shift){
    if(shift>=32){
        perror("\nShift must be less than 32");
        exit(1);
    }
    bignum result = calloc(lenA,sizeof(uint32_t));
    if(!result){
        allocError();
    }
    uint32_t carry = 0;
    uint64_t temp;
    for(int i=lenA-1;i>=0;i--){
        temp = ((uint64_t) a[i]) << shift; 
        result[i] = ((uint32_t) (temp & ULONG_MAX)) + carry;
        carry = (uint32_t) ((temp+(uint64_t)carry) >> 32);
    }
    if(carry){
        perror("\nOverflow error on bigNumLShift");
        exit(1);
    }
    return result;
}


void bigNumLShiftRe(bignum a,int lenA,int shift){
    bignum result = bigNumLShift(a,lenA,shift);
    memcpy(a,result,lenA*sizeof(uint32_t));
    free(result);
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
bignum bigNumMod(bignum a,int lenA,bignum n,int lenN){ //a % n
    //Long division
    //remainder must be of size lenN+1 so that it can handle an extra shift before being subtracted
    bignum remainder = (bignum) calloc(lenN+1,sizeof(uint32_t));
    if(!remainder){
        allocError();
    }
    int bits = lenA*32;
    for(int i=0;i<bits;i++){
        bigNumLShiftRe(remainder,lenN+1,1);
        int bit = ((a[i>>5] >> (31 - (i&31))) & 1);
        remainder[lenN] |= bit; //set LSB of curr to be the current bit in a
        uint8 cmpRes = bigNumCmp(remainder,lenN+1,n,lenN);
        if(cmpRes != LESS_THAN){ //If equal or greater
            bigNumSubRe(remainder,lenN+1,n,lenN);
            //Don't need to actually work out the quotient
        }
    }
    for(int i=1;i<=lenN;i++){ //move to the left 1 space as we are <lenN
        remainder[i-1] = remainder[i];
    }
    remainder = realloc(remainder,lenN*sizeof(uint32_t));
    return remainder; //Return the left over carry which will be the remainder
}


void bigNumModRe(bignum a,int lenA,bignum n,int lenN){ //a % n
    if(lenA<lenN){
        perror("\nInsufficient space for bigNumMod");
        exit(1);
    }
    bignum result = bigNumMod(a,lenA,n,lenN);
    memset(a,0,lenA*sizeof(uint32_t));
    memcpy(&a[lenA-lenN],result,lenN*sizeof(uint32_t));
    free(result);
}

//Returns size lenA
bignum bigNumDiv(bignum a,int lenA,bignum b,int lenB){ //returns the quotient
    //Long division
    bignum remainder = (bignum) calloc(lenB+1,sizeof(uint32_t));
    if(!remainder){
        allocError();
    }
    bignum quotient = (bignum) calloc(lenA,sizeof(uint32_t));
    if(!quotient){
        free(remainder);
        allocError();
    }
    int bits = lenA*32;
    for(int i=0;i<bits;i++){
        bigNumLShiftRe(remainder,lenB+1,1);
        int bit = ((a[i>>5] >> (31 - (i&31))) & 1);
        remainder[lenB] |= bit; //set LSB of curr to be the current bit in a
        uint8 cmpRes = bigNumCmp(remainder,lenB+1,b,lenB);
        if(cmpRes != LESS_THAN){ //Equal or greater than
            bigNumSubRe(remainder,lenB+1,b,lenB);
            quotient[i>>5] |= (1 << (31 - (i&31)));
            //Don't need to actually work out the quotient
        }
    }
    //Don't need to mess around with remainder
    return quotient; 
} 

void bigNumDivRe(bignum a,int lenA,bignum b,int lenB){
    bignum result = bigNumDiv(a,lenA,b,lenB);
    memcpy(a,result,lenA*sizeof(uint32_t));
    free(result);
}


bignum bigNumModAdd(bignum a,int lenA, bignum b, int lenB,bignum n,int lenN){
    int lenUnmodded = max(lenA,lenB)+1;
    bignum unmodded = bigNumAdd(a,lenA,b,lenB,lenUnmodded);
    bignum result = bigNumMod(unmodded,lenUnmodded,n,lenN);
    free(unmodded);
    return result;
}
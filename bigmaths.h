unsigned long* createBigNum(unsigned long *a, int len);
unsigned long* bigNumAdd(unsigned long *a,int lenA, unsigned long *b, int lenB, int lenDest);
unsigned long* multiAdd(unsigned long *a,int lenA,unsigned long *b, int lenB, unsigned long *c,int lenC, int lenDest);
unsigned long* bigNumMult(unsigned long *a,int lenA, unsigned long *b,int lenB,int lenDest);
unsigned long* bigNumModMult(unsigned long *a,int lenA, unsigned long *b,int lenB,int lenDest,int bitMod, int carryMult);
unsigned long* bigNumSub(unsigned long *a,int lenA, unsigned long *b,int lenB,int lenDest);
unsigned long* bigNumModSub(unsigned long *a,int lenA, unsigned long *b,int lenB,int lenDest,unsigned long *p,int lenP);
unsigned long* bigNumBitMod(unsigned long *a, int lenA,int bitMod,int carryMult, int lenDest);
unsigned long* bigNumModInv(unsigned long *a, int lenA,unsigned long *p, int lenP, int lenDest,int bitMod,int carryMult);
unsigned long* bigNumMultByLittle(unsigned long *a,int lenA, unsigned long littleNum,int lenDest);
unsigned long* bigNumModMultByLittle(unsigned long *a,int lenA,unsigned long littleNum,int lenDest,int bitMod, int carryMult);




void printBigNum(char *text, unsigned long *n, int lenN){
    printf("\n%s",text);
    for(int i=0;i<lenN;i++){
        printf(" %lu",n[i]);
    }
}
//NOTE - Numbers are stored with MSB at index 0

unsigned long* createBigNum(unsigned long *a, int len){ //MUST REMEMBER TO FREE IF USING THIS
    unsigned long* p = calloc(len,sizeof(unsigned long));
    if(!p){
        perror("\nCalloc error during addition bigNum creation");
        exit(1);
    }
    for(int i=0;i<len;i++){
        p[i] = a[i];
    }
    return p;
}



unsigned long* bigNumAdd(unsigned long *a,int lenA, unsigned long *b, int lenB, int lenDest){
    unsigned long long temp;
    int carry=0;
    if(lenDest < lenA || lenDest <lenB){
        
        perror("\nStorage destination for addition is too small");
        exit(1);
    }
    unsigned long *sum = calloc(lenDest,sizeof(unsigned long));
    if(!sum){
        perror("\nCalloc error during addition");
        exit(1);
    }
    int iA;
    int iB;
    for(int i= lenDest-1;i>-1;i--){
        iA = lenA - (lenDest-i);
        iB = lenB - (lenDest-i);
        
        unsigned long op1;
        unsigned long op2;
        if(iA<0) op1 = 0;
        else op1 = a[iA];
        if(iB<0) op2 = 0;
        else op2 = b[iB];
        temp = (unsigned long long) op1+op2+carry;
        carry = temp >> (sizeof(unsigned long)*8);
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

unsigned long* bigNumModAdd(unsigned long *a,int lenA, unsigned long *b,int lenB,int lenDest,int bitMod, int carryMult){
    unsigned long *addResult = bigNumAdd(a,lenA,b,lenB,lenDest+1);
    unsigned long *modResult = bigNumBitMod(addResult,lenDest+1,bitMod,carryMult,lenDest);
    free(addResult);
    return modResult;
}


unsigned long*  multiAdd(unsigned long *a,int lenA,unsigned long *b, int lenB, unsigned long *c,int lenC, int lenDest){
    unsigned long* sum1 = bigNumAdd(a,lenA,b,lenB,lenDest);
    unsigned long* sum2 = bigNumAdd(sum1,lenDest,c,lenC,lenDest);
    free(sum1);
    return sum2;
}

//TODO - Currently not actually any more efficient than long multiplication - see book to work out what to reuse
unsigned long* bigNumMult(unsigned long *a,int lenA, unsigned long *b,int lenB,int lenDest){ 
    /*a = [a1,a2]
    b = [b1,b2]
    a*b = [a1*b1,a1*b2+a2*b1,a2*b2]
    */
   ////printBigNum("Starting mult a: ",a,lenA);
   ////printBigNum("Starting mult b: ",b,lenB);
    int longest = max(lenA,lenB);
    if(lenDest<lenA+lenB){
        free(a); free(b);
        perror("\nStorage destination for multiplication is too small");
        exit(1);
    }
    unsigned long *product = calloc(lenDest,sizeof(unsigned long));
    if(!product){
        perror("\nCalloc error during multiplication");
        exit(1);
    }
    if(lenA>1 || lenB>1){
        if(longest & 1){
            if(longest==lenA){
                a = realloc(a,(lenA+1)*sizeof(unsigned long));
                memcpy(&a[1],a,lenA*sizeof(unsigned long));
                a[0] = 0;
                lenA++;
            }
            else{
                b = realloc(b,(lenB+1)*sizeof(unsigned long));
                memcpy(&b[1],b,lenB*sizeof(unsigned long));
                b[0] = 0;
                lenB++;
            }
            longest++;
        }
        if(lenA != lenB){ //Pad the start with zeroes
            if(longest == lenA){
                b = realloc(b,lenA*sizeof(unsigned long));
                memcpy(&b[lenA-lenB],b,lenB*sizeof(unsigned long));
                for(int i=0;i<(lenA-lenB);i++){
                    b[i] = 0;
                }
                lenB = longest;
            }
            else{
                a = realloc(a,lenB*sizeof(unsigned long));
                memcpy(&a[lenB-lenA],a,lenA*sizeof(unsigned long));
                for(int i=0;i<(lenB-lenA);i++){
                    a[i] = 0;
                }
                lenA = longest;
            }
        }

        unsigned long w[longest/2]; unsigned long x[longest/2]; unsigned long y[longest/2]; unsigned long z[longest/2];
        unsigned long wy1[longest/2];unsigned long wy2[longest/2]; unsigned long wz1[longest/2];unsigned long wz2[longest/2];
        unsigned long xy1[longest/2];unsigned long xy2[longest/2];unsigned long xz1[longest/2];unsigned long xz2[longest/2];

        memcpy(w, a, longest/2 * sizeof(unsigned long)); memcpy(x, &a[longest/2], longest/2 * sizeof(unsigned long)); 
        memcpy(y, b, longest/2 * sizeof(unsigned long)); memcpy(z, &b[longest/2], longest/2 * sizeof(unsigned long)); 
        
        unsigned long *wy = bigNumMult(w,longest/2,y,longest/2,longest);
        unsigned long *wz = bigNumMult(w,longest/2,z,longest/2,longest);
        unsigned long *xy = bigNumMult(x,longest/2,y,longest/2,longest);
        unsigned long *xz = bigNumMult(x,longest/2,z,longest/2,longest);

        memcpy(wy1, wy, longest/2 * sizeof(unsigned long)); memcpy(wy2, &wy[longest/2], longest/2 * sizeof(unsigned long));  
        memcpy(wz1, wz, longest/2 * sizeof(unsigned long)); memcpy(wz2, &wz[longest/2], longest/2 * sizeof(unsigned long));  
        memcpy(xy1, xy, longest/2 * sizeof(unsigned long)); memcpy(xy2, &xy[longest/2], longest/2 * sizeof(unsigned long));  
        memcpy(xz1, xz, longest/2 * sizeof(unsigned long)); memcpy(xz2, &xz[longest/2], longest/2 * sizeof(unsigned long));  

        
        unsigned long *pos1Temp = multiAdd(wy2,longest/2,xy1,longest/2,wz1,longest/2,(longest/2)+1); //Accounts for overflow
        unsigned long *pos2Temp = multiAdd(xy2,longest/2,wz2,longest/2,xz1,longest/2,(longest/2)+1);
        unsigned long *pos0 = bigNumAdd(wy1,longest/2,pos1Temp,1,longest/2); //Shouldn't overflow?
        unsigned long *pos1 = bigNumAdd(&pos1Temp[1],longest/2,pos2Temp,1,longest/2);
        unsigned long *pos2 = calloc(longest/2,sizeof(unsigned long));
        memcpy(pos2,&pos2Temp[1],longest/2*sizeof(unsigned long));
        free(pos1Temp);free(pos2Temp);
        int niceLength = (longest<<1);
        int quartered = niceLength>>2;
        int i = max(0,lenDest-niceLength);
        int diff = niceLength-lenDest;
        int j = diff%quartered;
        //printf("\ni %d",i);
        if(diff <quartered){ //Used to be <= 
            memcpy(&product[i],&pos0[j],(quartered-j)* sizeof(unsigned long));
            i+= quartered - j;
            //printf("\ni %d",i);
            j=0;
        }
        if(diff < 2*quartered){
            memcpy(&product[i],&pos1[j],(quartered-j)* sizeof(unsigned long));
            i+=quartered -j;
            //printf("\ni %d",i);
            j=0;
        }
        if(diff < 3*quartered){
            memcpy(&product[i],&pos2[j],(quartered-j) * sizeof(unsigned long));
            i+=quartered-j;
            //printf("\ni %d",i);
            j=0;
        }
        //printf("\ni %d j %d",i,j);
        memcpy(&product[i],&xz2[j],(quartered-j) * sizeof(unsigned long));
        ////printBigNum("Mult product: ",product,lenDest);

        free(wy);free(wz);free(xy);free(xz);
        free(pos0);free(pos1);free(pos2);
    }
    else{
        unsigned long long result = (unsigned long long)a[0]*b[0];
        //printf("\nMult result llu %llu ",result);
        unsigned long smallResult = (unsigned long) (result & 0xffffffffUL);
       // printf("\nSmall result %lu ",smallResult);
        unsigned long bigResult = (unsigned long) (result >> 32);
        //printf("\nBig result %lu ",bigResult);
        product[0] = bigResult;
        product[1] = smallResult;
        ////printBigNum("Product: ",product,2);
    }
    return product;
}

unsigned long* bigNumModMult(unsigned long *a,int lenA, unsigned long *b,int lenB,int lenDest,int bitMod, int carryMult){
    unsigned long *multResult = bigNumMult(a,lenA,b,lenB,lenDest*2);
    unsigned long *modResult = bigNumBitMod(multResult,lenDest*2,bitMod,carryMult,lenDest);
    free(multResult);
    return modResult;
}

unsigned long* bigNumBitMod(unsigned long *a, int lenA,int bitMod,int carryMult, int lenDest){
    unsigned long *product = calloc(lenDest,sizeof(unsigned long));
    if(!product){
        perror("\nCalloc error during mod");
        exit(1);
    }
    int i = (int)((float)lenA - (float)bitMod/32); //Do we need to mod the number, if so how many chunks do we need to mod
    if(!(bitMod%32)) i--; //Doesn't affect 25519
    if(i<0){
        memcpy(product,a,lenA*sizeof(unsigned long));
        return product;
    }
    unsigned long *carry, *realCarry,*addedCarry,doubleCarry;
    do{//Repeat until no more modding is require, 1 iteration on 0xfffff will require another mod
        doubleCarry = 0;
        int bitDepth = bitMod%32; //How far to go into the first chunk
        carry = calloc(i+1,sizeof(unsigned long)); //Allocate enough chunks
        if(!carry){
            perror("\nCalloc error during mod");
            exit(1);
        }
        for(int j=0;j<=i;j++){ //Go through all the chunks that are too big
            if(j==i){
                carry[j] = (unsigned long long) (a[j] >> bitDepth) + doubleCarry;
                a[j] -= (a[j] >> bitDepth) << bitDepth; //get rid of the big bit
            }
            else{
                 //Double carry ensures that the positioning of array elements is correct
                carry[j] = (unsigned long long) (a[j] >> bitDepth) + doubleCarry; //e.g. if you are doing bitDepth 31 and there is a {...2},{1} carry this should be represented as {0,2*2^1 + 1}
                doubleCarry = (a[j] - ((a[j] >> bitDepth)<<bitDepth)) * (1<<(32-bitDepth));
                a[j] = 0;
            }
        }
        int lenRealCarry = (!(bitMod%32))?2:1; //Doesn't work with addition and ternary operator
        lenRealCarry += i;
        realCarry = bigNumMultByLittle(carry,i+1,carryMult,lenRealCarry);

        addedCarry = bigNumAdd(realCarry,lenRealCarry,a,lenA,lenA);
        memcpy(a,addedCarry, lenA * sizeof(unsigned long));

        if(a[i] < pow(2,bitDepth) && (i==0 || a[i-1] == 0)) break;
        free(carry);free(realCarry);free(addedCarry);
        
    }
    while(true);
    memcpy(product,&a[lenA-lenDest],lenDest*sizeof(unsigned long));
    return product;
}



unsigned long* bigNumMultByLittle(unsigned long *a,int lenA, unsigned long littleNum,int lenDest){
    if(lenDest<lenA){
        perror("\nStorage destination for multiplication is too small");
        exit(1);
    }
    unsigned long *product = calloc(lenDest,sizeof(unsigned long));
    if(!product){
        perror("\nCalloc error during mutliplication");
        exit(1);
    }
    unsigned long thisChunk = 0;
    unsigned long carry = 0;
    int pI;
    for(int i=lenA-1;i>-1;i--){ //TODO - doesn't take into account lenDest and so overflows on any possible overflow
        pI = lenDest - (lenA-i);
        unsigned long long result = (unsigned long long)a[i]*littleNum + carry;
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

unsigned long* bigNumModMultByLittle(unsigned long *a,int lenA, unsigned long littleNum,int lenDest,int bitMod, int carryMult){
    unsigned long *multResult = bigNumMultByLittle(a,lenA,littleNum,lenDest*2);
    unsigned long *modResult = bigNumBitMod(multResult,lenDest*2,bitMod,carryMult,lenDest);
    free(multResult);
    return modResult;
}

unsigned long* bigNumSub(unsigned long *a,int lenA, unsigned long *b,int lenB,int lenDest){
    long long temp;
    int carry=0;
    if(lenB>lenA){
        perror("\nFirst argument must be smaller than second for subtraction");
        exit(1);
    }
    unsigned long *result = calloc(lenDest,sizeof(unsigned long));
    if(!result){
        perror("\nCalloc error during subtraction");
        exit(1);
    }
    int iA;
    int iB;
    for(int i= lenDest-1;i>-1;i--){
        iA = lenA - (lenDest-i);
        iB = lenB - (lenDest-i);
        unsigned long op1;
        unsigned long op2;
        if(iA<0) op1 = 0;
        else op1 = a[iA];
        if(iB<0) op2 = 0;
        else op2 = b[iB];
        temp = (long long) op1-op2+carry;
        carry = temp >> (sizeof(unsigned long)*8); //check for negative number
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

unsigned long* bigNumModSub(unsigned long *a,int lenA, unsigned long *b,int lenB,int lenDest,unsigned long *p,int lenP){
    //Answer is guaranteed in range (2^bitmod-carryMult,-(2^bitmod-carryMult))
    //As the inputs have already been modded
    // -25 mod 256 = 231 = 256 -25
    //
    long long temp=0,carry=0;
    unsigned long *result = calloc(lenDest,sizeof(unsigned long));
    if(!result){
        perror("\nCalloc error during subtraction");
        exit(1);
    }
    int iA;
    int iB;
    for(int i= lenDest-1;i>-1;i--){
        iA = lenA - (lenDest-i);
        iB = lenB - (lenDest-i);
        
        unsigned long op1;
        unsigned long op2;
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
                unsigned long notted,chunk;
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
                        notted -= (pow(2,(31-(j & 31)))-1); //Remove the following bits which have been turned to 1s
                        // 0110110111
                        // 0110110000
                        // 1001001111
                        // 1001000000
                    }
                    
                    if(j%32 != 31){
                        result[j >> 5] = notted + (result[j>>5]&((unsigned long)pow(2,(31-(j & 31)))-1));
                        j-= (j&31)+1;
                    }
                    else{
                        result[j >> 5] = notted; 
                        j-= 32;
                    }
                    
                }
                unsigned long *modded = bigNumSub(p,lenP,result,lenDest,lenDest); //Will be positive
                memcpy(result,modded,lenDest*sizeof(unsigned long));
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

unsigned long* bigNumModInv(unsigned long *a, int lenA,unsigned long *p, int lenP, int lenDest,int bitMod,int carryMult){
    //inv a = a^p-2
    //Constant time as p-2 is constant (for any given curve)

    //Not tested but follows the paper exactly
    int i = 1,bit;
    bool started = false;
    unsigned long *pCpy = calloc(lenP,sizeof(unsigned long));
    unsigned long *r = calloc(lenA,sizeof(unsigned long));
    
    unsigned long *temp1,*temp2;
    unsigned long *modTemp1,*modTemp2;

    if(!r || !pCpy){
        perror("\nCalloc error during inverse");
        exit(1);
    }
    r[lenA-1] = 1;
    memcpy(pCpy,p,lenP*sizeof(unsigned long));
    while(i<256){  //MSB to LSB
        temp1 = bigNumMult(r,lenA,r,lenA,lenA*2);
        modTemp1 = bigNumBitMod(temp1,lenA*2,bitMod,carryMult,lenA);
        memcpy(r,modTemp1,lenA * sizeof(unsigned long));
        free(temp1); free(modTemp1);

        bit = (pCpy[i >> 5] >> (31-(i & 31))) & 1;
        if(bit){
            temp2 = bigNumMult(r,lenA,a,lenA,lenA*2);
            modTemp2 = bigNumBitMod(temp2,lenA*2,bitMod,carryMult,lenA);
            memcpy(r,modTemp2,lenA * sizeof(unsigned long));
            free(temp2); free(modTemp2);
        }
        i++;
        
    }
    return r;
}
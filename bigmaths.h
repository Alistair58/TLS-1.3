unsigned long* createBigNum(unsigned long *a, int len);
unsigned long* bigNumAdd(unsigned long *a,int lenA, unsigned long *b, int lenB, int lenDest);
unsigned long* multiAdd(unsigned long *a,int lenA,unsigned long *b, int lenB, unsigned long *c,int lenC, int lenDest);
unsigned long* bigNumMult(unsigned long *a,int lenA, unsigned long *b,int lenB,int lenDest);
unsigned long* bigNumModMult(unsigned long *a,int lenA, unsigned long *b,int lenB,int lenDest,int bitMod, int carryMult);
unsigned long* bigNumSub(unsigned long *a,int lenA, unsigned long *b,int lenB,int lenDest);
unsigned long* bigNumModSub(unsigned long *a,int lenA, unsigned long *b,int lenB,int lenDest,unsigned long *p,int lenP);
unsigned long* bigNumBitMod(unsigned long *a, int lenA,int bitMod,int carryMult, int lenDest);
unsigned long* bigNumModInv(unsigned long *a, int lenA,unsigned long *p, int lenP, int lenDest,int bitMod,int carryMult);
unsigned long* bigNumMultByLittle(unsigned long *a,int lenA, int littleNum,int lenDest);
unsigned long* bigNumModMultByLittle(unsigned long *a,int lenA, int littleNum,int lenDest,int bitMod, int carryMult);




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
        temp = op1+op2+carry;
        carry = temp >> (sizeof(unsigned long)*8);
        if(carry>0){
            temp %= (unsigned long) pow(2,sizeof(unsigned long)*8);
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

        
        unsigned long *pos1 = multiAdd(wy2,longest/2,xy1,longest/2,wz1,longest/2,longest/2); //TODO need to put as longest and then put overflow into wy1 or see if it works without
        unsigned long *pos2 = multiAdd(xy2,longest/2,wz2,longest/2,xz1,longest/2,longest/2);
        int niceLength = (longest*2);
        int quartered = niceLength>>2; //Just for readability
        int i = 0;
        int diff = niceLength-lenDest;
        int j = diff%quartered;
        //printf("\ni %d",i);
        if(diff <=quartered){
            memcpy(&product[i],&wy1[j],(quartered-j)* sizeof(unsigned long));
            i+= quartered - j;
            //printf("\ni %d",i);
            j=0;
        }
        if(diff <= 2*quartered){
            memcpy(&product[i],&pos1[j],(quartered-j)* sizeof(unsigned long));
            i+=quartered -j;
            //printf("\ni %d",i);
            j=0;
        }
        if(diff <= 3*quartered){
            memcpy(&product[i],&pos2[j],(quartered-j) * sizeof(unsigned long));
            i+=quartered-j;
            //printf("\ni %d",i);
            j=0;
        }
        //printf("\ni %d j %d",i,j);
        memcpy(&product[i],&xz2[j],(quartered-j) * sizeof(unsigned long));
        ////printBigNum("Mult product: ",product,lenDest);

        free(wy);free(wz);free(xy);free(xz);
        free(pos1);free(pos2);
    }
    else{
        unsigned long long result = (unsigned long long)a[0]*b[0];
        //printf("\nMult result llu %llu ",result);
        unsigned long smallResult = (unsigned long) (result & 0xffffffffUL);
       // printf("\nSmall result %lu ",smallResult);
        unsigned long bigResult = (unsigned long) (result >> sizeof(unsigned long) * 8);
        //printf("\nBig result %lu ",bigResult);
        product[0] = bigResult;
        product[1] = smallResult;
        ////printBigNum("Product: ",product,2);
    }
    return product;
}

unsigned long* bigNumModMult(unsigned long *a,int lenA, unsigned long *b,int lenB,int lenDest,int bitMod, int carryMult){
    unsigned long *multResult = bigNumMult(a,lenA,b,lenB,lenDest*2);
    //Multiplication works - tested
    //Mod might work - hard to tell
    unsigned long *modResult = bigNumBitMod(multResult,lenDest*2,bitMod,carryMult,lenDest);
    //((1313454604*2^224+1181031284*2^192+3085259558*2^160+1799379996*2^128+2147483647*2^96+1724*2^64+2942037868*2^32+3350119311)^2)%(2^255-19)
    //It says first digit is 1558472168 but my first digit is 369177677 - need to test with smaller numbers
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
                carry[j] = (a[j] >> bitDepth) + doubleCarry;
                a[j] -= (carry[j]-doubleCarry) << bitDepth;
            }
            else{
                 //Double carry ensures that the positioning of array elements is correct
                carry[j] = (a[j] >> bitDepth) + doubleCarry; //e.g. if you are doing bitDepth 31 and there is a {...,2} {1} carry this should be represented as {0,21}
                doubleCarry = a[j] - (a[j] >> bitDepth);
                a[j] = 0;
            }
        }

        realCarry = bigNumMultByLittle(carry,i+1,carryMult,i+2);

        addedCarry = bigNumAdd(realCarry,i+2,a,lenA,lenA);
        memcpy(a,addedCarry, lenA * sizeof(unsigned long));

        if(a[i] < pow(2,bitDepth) && (i==0 || a[i-1] == 0)) break;
        free(carry);free(realCarry);free(addedCarry);
        
    }
    while(true);
    memcpy(product,&a[lenA-lenDest],lenDest*sizeof(unsigned long));
    return product;
}



unsigned long* bigNumMultByLittle(unsigned long *a,int lenA, int littleNum,int lenDest){
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
    for(int i=lenA-1;i>-1;i--){
        pI = lenDest - (lenA-i);
        unsigned long long result = a[i]*littleNum;
        thisChunk = (result+carry) % (unsigned long) pow(256,sizeof(unsigned long));
        carry = result >> sizeof(unsigned long) * 8;  
        product[pI] = thisChunk;
        if(i==0 && carry){
            free(product);
            perror("\nMultiplication overflow");
            exit(1);
        }
    }
    return product;
    
}

unsigned long* bigNumModMultByLittle(unsigned long *a,int lenA, int littleNum,int lenDest,int bitMod, int carryMult){
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
        temp = op1-op2+carry;
        carry = temp >> (sizeof(long long)*8 -1); //check for negative number
        if(carry!=0){
            temp %= (unsigned long) pow(2,sizeof(unsigned long)*8);
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
    long long temp;
    bool carry,prevCarry;
    //printBigNum("a",a,lenA);
    //printBigNum("b",b,lenB);
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
        
        temp = (long long) op1- (long long) op2; //If you don't cast, you don't get any negatives
        prevCarry = carry;
        carry = (long long) (temp+((carry)?-1:0)) >> (sizeof(unsigned long)*8); //If this is negative or any previous digits have been negative
        if(carry){
            //if(i!=0){
                temp += p[i]; //Make it positive
           // }
            /*else{
                /*256 - 19 = 237
                512 mod (237) = 38 (=2*19)
                -512 mod(237) = 237-38
                temp *= -1;
                result[i] = temp;
                printBigNum("pre mod result",result,lenDest);
                unsigned long* trueResult = bigNumSub(p,lenP,result,lenDest,lenDest);
                memcpy(result,trueResult,lenDest*sizeof(unsigned long));
                free(trueResult);
            }*/
        }
        else if(prevCarry)temp--;
        result[i] = temp;
    }
    //printBigNum("Result: ",result,lenDest);
    return result;
}

unsigned long* bigNumModInv(unsigned long *a, int lenA,unsigned long *p, int lenP, int lenDest,int bitMod,int carryMult){
    //inv a = a^p-2
    int chunk = lenP - 1;
    int chunkDepth = 0;
    bool started = false;
    unsigned long *pCpy = calloc(lenP,sizeof(unsigned long));
    unsigned long *inv = calloc(lenA,sizeof(unsigned long));
    unsigned long *r = calloc(lenA,sizeof(unsigned long));
    
    unsigned long *temp1,*temp2;
    unsigned long *modTemp1,*modTemp2;
    if(!r || !inv || !pCpy){
        perror("\nCalloc error during inverse");
        exit(1);
    }
    memcpy(r,a,lenA*sizeof(unsigned long));
    memcpy(pCpy,p,lenP*sizeof(unsigned long));
    while(chunk > -1){ 
        if(pCpy[chunk] & 1 ==1){
            if(!started){
                started = true;
                memcpy(inv,r,sizeof(unsigned long)*lenA);
            }
            else{
                temp1 = bigNumMult(inv,lenA,r,lenA,lenA*2);
                modTemp1 = bigNumBitMod(temp1,lenA*2,bitMod,carryMult,lenA);
                memcpy(inv,modTemp1,lenA * sizeof(unsigned long));
                free(temp1); free(modTemp1);
            }
        }
        pCpy[chunk] >>= 1;
        chunkDepth++;
        if(chunkDepth==32){
            chunk--;
            chunkDepth=0;
        }
        temp2 = bigNumMult(r,lenA,r,lenA,lenA*2);
        modTemp2 = bigNumBitMod(temp2,lenA*2,bitMod,carryMult,lenA);
        memcpy(r,modTemp2,lenA * sizeof(unsigned long));
        free(temp2); free(modTemp2);
    }
    return inv;
}
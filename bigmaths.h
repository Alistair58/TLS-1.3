unsigned long* createBigNum(unsigned long *a, int len);
unsigned long* bigNumAdd(unsigned long *a,int lenA, unsigned long *b, int lenB, int lenDest);
unsigned long* multiAdd(unsigned long *a,int lenA,unsigned long *b, int lenB, unsigned long *c,int lenC, int lenDest);
unsigned long* bigNumMult(unsigned long *a,int lenA, unsigned long *b,int lenB,int lenDest);
unsigned long* bigNumModMult(unsigned long *a,int lenA, unsigned long *b,int lenB,int lenDest,int bitMod, int carryMult);
unsigned long* bigNumSub(unsigned long *a,int lenA, unsigned long *b,int lenB,int lenDest);
unsigned long* bigNumBitMod(unsigned long *a, int lenA,int bitMod,int carryMult, int lenDest);
unsigned long* bigNumBitModNeg(long long *a, int lenA,int bitMod,int carryMult, int lenDest);
unsigned long* bigNumModInv(unsigned long *a, int lenA,unsigned long *p, int lenP, int lenDest,int bitMod,int carryMult);
unsigned long* bigNumMultByLittle(unsigned long *a,int lenA, int littleNum,int lenDest);
unsigned long* bigNumModMultByLittle(unsigned long *a,int lenA, int littleNum,int lenDest,int bitMod, int carryMult);



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
        iA = lenA - (lenDest-1-i);
        iB = lenB - (lenDest-1-i);
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

        
        unsigned long *pos1 = multiAdd(wy2,longest/2,xy1,longest/2,wz1,longest/2,longest);
        unsigned long *pos2 = multiAdd(xy2,longest/2,wz2,longest/2,xz1,longest/2,longest);
        
        int niceLength = (longest*2);
        int quartered = niceLength>>2; //Just for readability
        int i = 0;
        int diff = niceLength-lenDest;
        int j = diff%quartered;
        if(diff <=quartered){
            memcpy(&product[i],&wy1[j],(quartered-j)* sizeof(unsigned long));
            i+= quartered - j;
            j=0;
        }
        if(diff <= 2*quartered){
            memcpy(&product[i],&pos1[j],(quartered-j)* sizeof(unsigned long));
            i+=quartered -j;
            j=0;
        }
        if(diff <= 3*quartered){
            memcpy(&product[i],&pos2[j],(quartered-j) * sizeof(unsigned long));
            i+=quartered-j;
            j=0;
        }
        memcpy(&product[i],&xz2[j],(quartered-j) * sizeof(unsigned long));
        

        free(wy);free(wz);free(xy);free(xz);
        free(pos1);free(pos2);
    }
    else{
        unsigned long long result = a[0]*b[0];
        unsigned long smallResult = result % (unsigned long) pow(256,sizeof(unsigned long));
        unsigned long bigResult = result >> sizeof(unsigned long) * 8;
        product[0] = bigResult;
        product[1] = smallResult;
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
    int i = lenA - bitMod/32;
    if(i<0){
        memcpy(product,a,lenA*sizeof(unsigned long));
        return product;
    }
    while(i>=0){
        int bitDepth = bitMod%32;
        unsigned long *carry = calloc(i+1,sizeof(unsigned long));
        if(!carry){
            perror("\nCalloc error during mod");
            exit(1);
        }
        for(int j=i;i>-1;j--){
            carry[i] = a[i] >> bitDepth;
            a[i] = a[i] % (unsigned long) pow(2,bitDepth);
            bitDepth = 0;
        }
        unsigned long *result = bigNumMultByLittle(carry,i+1,carryMult,lenDest);
        a = bigNumAdd(carry,i+1,a,lenA,lenA);
        for(int k=0;k<lenA;k++){
            if(a[k]==0) lenA--;
        }
        int i = lenA - bitMod/32;
        free(carry);
        free(result);
    }
    memcpy(product,&a[lenA-lenDest],lenDest*sizeof(unsigned long));
    return product;
}

unsigned long* bigNumBitModNeg(long long *a, int lenA,int bitMod,int carryMult, int lenDest){
    unsigned long *product = calloc(lenDest,sizeof(unsigned long));
    if(!product){
        perror("\nCalloc error during mod");
        exit(1);
    }
    int i = lenA - bitMod/32;
    if(i<0){
        memcpy(product,a,lenA*sizeof(unsigned long));
        return product;
    }
    while(i>=0){
        int bitDepth = bitMod%32;
        unsigned long *carry = calloc(i+1,sizeof(unsigned long));
        if(!carry){
            perror("\nCalloc error during mod");
            exit(1);
        }
        for(int j=i;i>-1;j--){
            carry[i] = a[i] >> bitDepth;
            a[i] = a[i] % (unsigned long) pow(2,bitDepth);
            bitDepth = 0;
        }
        unsigned long *result = bigNumMultByLittle(carry,i+1,carryMult,lenDest);
        a = bigNumAdd(carry,i+1,a,lenA,lenA);
        for(int k=0;k<lenA;k++){
            if(a[k]==0) lenA--;
        }
        int i = lenA - bitMod/32;
        free(carry);
        free(result);
    }
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
    for(int i=lenA-1;i>-1;i--){
        unsigned long long result = a[i]*littleNum;
        thisChunk = result+carry % (unsigned long) pow(256,sizeof(unsigned long));
        carry = result >> sizeof(unsigned long) * 8;  
        product[i] = thisChunk;
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
        iA = lenA - (lenDest-1-i);
        iB = lenB - (lenDest-1-i);
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

unsigned long* bigNumModSub(unsigned long *a,int lenA, unsigned long *b,int lenB,int lenDest,int bitMod, int carryMult){
    long long temp;
    int carry=0;
    long long *result = calloc(lenDest+1,sizeof(unsigned long)); //Can have negative values
    if(!result){
        perror("\nCalloc error during subtraction");
        exit(1);
    }
    int iA;
    int iB;
    for(int i= lenDest-1;i>-1;i--){
        iA = lenA - (lenDest-1-i);
        iB = lenB - (lenDest-1-i);
        unsigned long op1;
        unsigned long op2;
        if(iA<0) op1 = 0;
        else op1 = a[iA];
        if(iB<0) op2 = 0;
        else op2 = b[iB];
        temp = op1-op2+carry;
        carry = temp >> (sizeof(long long)*8 -1); //check for negative number
        if(carry!=0){
            if(i!=0){
                temp %= (unsigned long) pow(2,sizeof(unsigned long)*8);
            }
        }    
        result[i] = temp;
    }
    unsigned long *modResult = bigNumBitModNeg(result,lenDest+1,bitMod,carryMult,lenDest);
    free(result);
    return modResult;
}

unsigned long* bigNumModInv(unsigned long *a, int lenA,unsigned long *p, int lenP, int lenDest,int bitMod,int carryMult){
    //inv a = a^p-2
    int chunk = lenP - 1;
    bool started = false;
    unsigned long *inv = calloc(lenP,sizeof(unsigned long));
    unsigned long *r = calloc(lenP,sizeof(unsigned long));
    if(!r){
        perror("\nCalloc error during inverse");
        exit(1);
    }
    while(chunk > 0){ 
        if(p[chunk] & 1 ==1){
            if(!started){
                started = true;
                memcpy(inv,r,sizeof(unsigned long)*lenP);
            }
            else{
                unsigned long *temp = bigNumMult(inv,lenA,r,lenP,lenP*2);
                unsigned long *modTemp = bigNumBitMod(temp,lenP*2,bitMod,carryMult,lenP);
                memcpy(inv,modTemp,lenP * sizeof(unsigned long));
                free(temp); free(modTemp);
            }
        }
        p[chunk] >>= 1;
        if(p[chunk]==0) chunk--;
        unsigned long *temp = bigNumMult(r,lenP,r,lenP,lenP*2);
        unsigned long *modTemp = bigNumBitMod(temp,lenP*2,bitMod,carryMult,lenP);
        memcpy(r,modTemp,lenP * sizeof(unsigned long));
        free(temp); free(modTemp);
    }
}
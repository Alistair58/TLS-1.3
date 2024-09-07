unsigned long* createBigNum(unsigned long *a, int len);
unsigned long* bigNumAdd(unsigned long *a,int lenA, unsigned long *b, int lenB, int lenDest);
unsigned long* multiAdd(unsigned long *a,int lenA,unsigned long *b, int lenB, unsigned long *c,int lenC, int lenDest);
unsigned long* bigNumMult(unsigned long *a,int lenA, unsigned long *b,int lenB,int lenDest);


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

//NOTE - Numbers are stored with MSB at index 0

unsigned long* bigNumAdd(unsigned long *a,int lenA, unsigned long *b, int lenB, int lenDest){
    unsigned long long temp;
    int carry=0;
    if(lenDest < lenA || lenDest <lenB){
        perror("\nStorage destination for addition is too small");
        exit(1);
    }
    unsigned long *sum = calloc(lenDest,sizeof(unsigned long));
    printf("\nSum pointer %p ",sum);
    if(!sum){
        perror("\nCalloc error during addition");
        exit(1);
    }
    for(int i= lenDest-1;i>-1;i--){
        unsigned long op1;
        unsigned long op2;
        if(i>=lenA) op1 = 0;
        else op1 = a[i];
        if(i>=lenB) op2 = 0;
        else op2 = b[i];
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

unsigned long*  multiAdd(unsigned long *a,int lenA,unsigned long *b, int lenB, unsigned long *c,int lenC, int lenDest){
    unsigned long* sum1 = bigNumAdd(a,lenA,b,lenB,lenDest);
    unsigned long* sum2 = bigNumAdd(sum1,lenDest,c,lenC,lenDest);
    free(sum1);
    return sum2;
}

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
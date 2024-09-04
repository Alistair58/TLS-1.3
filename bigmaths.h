int bigNumAdd(unsigned long *a,int lenA, unsigned long *b, int lenB, unsigned long *dest, int lenDest);
int multiAdd(unsigned long *a,int lenA,unsigned long *b, int lenB, unsigned long *c,int lenC, unsigned long *dest, int lenDest);
int bigNumMult(unsigned long *a,int lenA, unsigned long *b,int lenB, unsigned long *dest,int lenDest);

//NOTE - Numbers are stored with MSB at index 0

int bigNumAdd(unsigned long *a,int lenA, unsigned long *b, int lenB, unsigned long *dest, int lenDest){
    unsigned long long temp;
    int carry=0;
    if(lenDest < lenA || lenDest <lenB){
        perror("\nStorage destination for addition is too small");
        exit(1);
    }
    for(int i= max(lenA,lenB);i>-1;i--){
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
                printf("\nAddition overflow");
                return 0;
            }
        }    
        dest[i] = temp;
    }
    return 1;
}

int multiAdd(unsigned long *a,int lenA,unsigned long *b, int lenB, unsigned long *c,int lenC, unsigned long *dest, int lenDest){
    int res1 = bigNumAdd(a,lenA,b,lenB,dest,lenDest);
    if(!res1) return res1;
    int res2 = bigNumAdd(dest,lenDest,c,lenC,dest,lenDest);
    if(!res2) return res2;
    printf(" %lu + %lu + %lu = %lu ",a[0],b[0],c[0],dest[0]);
    return 1;
}

int bigNumMult(unsigned long *a,int lenA, unsigned long *b,int lenB, unsigned long *dest,int lenDest){ 
    /*a = [a1,a2]
    b = [b1,b2]
    a*b = [a1*b1,a1*b2+a2*b1,a2*b2]
    */
    
    int longest = max(lenA,lenB);
    if(lenDest<lenA+lenB){
        printf("Multiplication destination is too small");
        return 0;
    }
    printf("\nlenA %d lenB %d",lenA,lenB);
    if(lenA>1 || lenB>1){
        printf("aquel");
        if(lenA != lenB){ //Pad the start with zeroes
            if(longest == lenA){ //Can't do it with pointers as a and b might be arrays and haven't been mallocd
                unsigned long temp[lenA];
                for(int i=0;i<lenA;i++){
                    temp[i] = 0; //I can't find an easy way to initialise a variable sized array
                }
                memcpy(&temp[lenA-lenB],b,lenB*sizeof(unsigned long));
                printf("there");
                b = temp;
            }
            else{
                unsigned long temp[lenB];
                for(int i=0;i<lenB;i++){
                    temp[i] = 0; //I can't find an easy way to initialise a variable sized array
                }
                memcpy(&temp[lenB-lenA],a,lenA*sizeof(unsigned long));
                a = temp;
            }
        }
        unsigned long w[longest/2]; unsigned long x[longest/2]; unsigned long y[longest/2]; unsigned long z[longest/2];
        memcpy(w, a, longest/2 * sizeof(unsigned long)); memcpy(x, &a[longest/2], longest/2 * sizeof(unsigned long)); 
        memcpy(y, b, longest/2 * sizeof(unsigned long)); memcpy(z, &b[longest/2], longest/2 * sizeof(unsigned long)); 
        unsigned long wy[longest];unsigned long wz[longest];unsigned long xy[longest];unsigned long xz[longest];
        unsigned long wy1[longest/2];unsigned long wy2[longest/2]; unsigned long wz1[longest/2];unsigned long wz2[longest/2];
        unsigned long xy1[longest/2];unsigned long xy2[longest/2];unsigned long xz1[longest/2];unsigned long xz2[longest/2];
        bigNumMult(w,longest/2,y,longest/2,wy,longest);
        bigNumMult(w,longest/2,z,longest/2,wz,longest);
        bigNumMult(x,longest/2,y,longest/2,xy,longest);
        bigNumMult(x,longest/2,z,longest/2,xz,longest);
        memcpy(wy1, wy, longest/2 * sizeof(unsigned long)); memcpy(wy2, &wy[longest/2], longest/2 * sizeof(unsigned long));  
        memcpy(wz1, wz, longest/2 * sizeof(unsigned long)); memcpy(wz2, &wz[longest/2], longest/2 * sizeof(unsigned long));  
        memcpy(xy1, xy, longest/2 * sizeof(unsigned long)); memcpy(xy2, &xy[longest/2], longest/2 * sizeof(unsigned long));  
        memcpy(xz1, xz, longest/2 * sizeof(unsigned long)); memcpy(xz2, &xz[longest/2], longest/2 * sizeof(unsigned long));  

        unsigned long *pos1 = calloc(longest,sizeof(unsigned long));
        unsigned long *pos2 = calloc(longest,sizeof(unsigned long));
        multiAdd(wy2,longest/2,xy1,longest/2,wz1,longest/2,pos1,longest);
        multiAdd(xy2,longest/2,wz2,longest/2,xz1,longest/2,pos2,longest);
        
        printf("\nlenDest %d xz1 %lu xz2 %lu ",lenDest,xz1[0],xz2[0]);
        int niceLength = (longest*2);
        int quartered = niceLength>>2; //Just for readability
        int i = quartered;
        if(niceLength-lenDest <= 0) memcpy(dest,wy1,quartered* sizeof(unsigned long));
        else i=(quartered%2==0)?0:1;
        if(niceLength -quartered-lenDest <= 0){
            memcpy(&dest[i],pos1,quartered* sizeof(unsigned long));
            i+=quartered;
        }
        else i=(lenDest%2==0)?0:1;
        if(niceLength-quartered*2-lenDest <= 0){
            memcpy(&dest[i],pos2,quartered * sizeof(unsigned long));
            i+=quartered;
        }
        else i=(lenDest%2==0)?0:1;
        memcpy(&dest[i],xz2,quartered * sizeof(unsigned long));
        
    }
    else{
        unsigned long long result = a[0]*b[0];
        unsigned long smallResult = result % (unsigned long) pow(256,sizeof(unsigned long));
        unsigned long bigResult = result >> sizeof(unsigned long) * 8;
        printf("\nmult %lu * %lu = %lu smallResult %lu bigResult %lu",a[0],b[0],result,smallResult,bigResult);
        dest[0] = bigResult;
        dest[1] = smallResult;
    }
    return 1;

}
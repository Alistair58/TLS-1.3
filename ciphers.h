#include <sysinfoapi.h>
#include "bigmaths.h"
int randomNumber(unsigned long *bigIntArr, int chunks,unsigned long *n);
int ECDH(unsigned long *bigIntArr,struct CurveGroupParams params, unsigned long *q);
int ECPointAddition(unsigned long** p1,unsigned long** p2, unsigned long** dest,bool same,struct CurveGroupParams params);


int randomNumber(unsigned long *bigIntArr, int chunks,unsigned long *n){  //A chunk is 32 bits
    int time = 2500;
    unsigned long long product; //Unsigned long would mean the mod would never work and would output 0
    int x;
    int y;
    POINT point;
    ULONGLONG tickCount = GetTickCount64();
    ULONGLONG targetTime = tickCount +time; //5 seconds later
    product = (tickCount) % (unsigned long)(pow(256,4)); //mod 4 bytes
    int chunkWriteCount = 1;
    while(tickCount<= targetTime){
        GetCursorPos(&point);
        x = point.x;
        y = point.y;
        if(product){ //If not zero
            product = (product * x * y)% (unsigned long)(pow(256,4));
        }
        else{
            product += x + y;
        }
        if((time-(targetTime - tickCount)) >= time*(chunkWriteCount/(chunks+2))){ //+2 means that chunks aren't written at the end and so can't be missed
            if(chunkWriteCount == 1  && product >= n[chunkWriteCount-1]){
                while(product >= n[chunkWriteCount-1]){
                    product = (product * x * y)% (unsigned long)(pow(256,4)); //Keep generating until its smaller
                }
            }  
            bigIntArr[chunkWriteCount-1] = product;
            chunkWriteCount ++;
            if(chunkWriteCount == chunks) break;
        }
        tickCount = GetTickCount64();
    }
    
    return 1;
}

int ECDH(unsigned long *bigIntArr,struct CurveGroupParams params, unsigned long *q){
    int chunk = 0;
    bool started = false;
    unsigned long *p[2];
    p[0] = malloc(sizeof(unsigned long)*8); p[1] = malloc(sizeof(unsigned long)*8);
    unsigned long *r[2];
    r[0] = malloc(sizeof(unsigned long)*8); r[1] = malloc(sizeof(unsigned long)*8);
    memcpy(r[0],secp256rParams.G[0],sizeof(r[0])); memcpy(r[1],secp256rParams.G[1],sizeof(r[1]));
    while(chunk < 8){ //Binary exponentiation of point addition
        if(q[chunk] & 1 ==1){
            if(!started){
                started = 0;
                memcpy(r,p,sizeof(p));
            }
            else{
                ECPointAddition(r,p,p,false,params);
            }
        }
        q[chunk] >>= 1;
        if(q[chunk]==0) chunk++;
        ECPointAddition(r,r,r,true,params);
    }
    free(p[0]);free(p[1]);
    free(r[0]);free(r[1]);
    return 1;
}

int ECPointAddition(unsigned long** p1,unsigned long** p2, unsigned long** dest,bool same,struct CurveGroupParams params){
    unsigned long *m = malloc(sizeof(unsigned long)*8);
    unsigned long *temp = malloc(sizeof(unsigned long)*8);
    unsigned long three = 3;
    /*if(same){
        bigNumMult(p1[0],&three,m); 
        bigNumAdd(m,params.a,m);
        bigNumShift(p1[1],1,temp);
        bigNumDiv(m,temp,m); //m = (3x_0^2 + a) / (2y_0)
    }
    else{
        bigNumSub(p2[1],p1[1],m);
        bigNumSub(p2[0],p1[0],temp);
        bigNumDiv(m,temp,m); //m = (y_1-y_0)/(x_1-x_0)
    }
    bigNumMult(m,m,temp);
    bigNumSub(temp,p1[0],temp);
    bigNumSub(temp,p2[0],dest[0]);

    bigNumSub(p1[0],p2[0],temp);
    bigNumMult(m,temp,temp);
    bigNumSub(temp,p1[1],dest[1]);*/

    free(m);
    free(temp);

    return 1;
}
#include "random.h"
#include <sysinfoapi.h>
#include <windows.h>
#include <winuser.h>
#include <math.h>
#include <stdint.h>

int randomNumber(bignum bigIntArr, int chunks,bignum n){  //A chunk is 32 bits
    int time = 1000;
    uint64_t product; //uint32_t would mean the mod would never work and would output 0
    int x;
    int y;
    POINT point;
    ULONGLONG tickCount = GetTickCount64();
    ULONGLONG targetTime = tickCount +time; //2.5 seconds later
    product = (tickCount) % (uint32_t)(pow(256,sizeof(uint32_t))-1); //mod 4 bytes
    int chunkWriteCount = 1;
    uint32_t mod;
    if(n==NULL) mod = pow(256,sizeof(uint32_t))-1; //Can only get up to 256^uint32_t -2
    else mod = n[0];
    while(chunkWriteCount<=chunks){
    
        GetCursorPos(&point);
        x = point.x;
        y = point.y;
        if(product && x && y){ //If not zero
            product = (product * x * y)% mod;
        }
        else{
            product += x + y;
        }
        if((targetTime - tickCount) <= time*((float)(chunks-chunkWriteCount)/(chunks+1))){ //+1 means chunks aren't written at start or end
            int timeProg = (int)(targetTime - tickCount);
            //TIME PROG PRINTS OUT ZERO
            int timeNeeded = (int)time*((float)(chunks-chunkWriteCount+1)/(chunks+1));
            bigIntArr[chunkWriteCount-1] = product;
            chunkWriteCount ++;
            mod = pow(256,sizeof(uint32_t))-1;
            if(chunkWriteCount > chunks) break;
        }
        tickCount = GetTickCount64();
    }
    
    return 0;
}

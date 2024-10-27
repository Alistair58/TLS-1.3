#include <sysinfoapi.h>
#include <windows.h>
#include <winuser.h>
#include <math.h>

int randomNumber(unsigned long *bigIntArr, int chunks,unsigned long *n);

int randomNumber(unsigned long *bigIntArr, int chunks,unsigned long *n){  //A chunk is 32 bits
    int time = 1000;
    unsigned long long product; //Unsigned long would mean the mod would never work and would output 0
    int x;
    int y;
    POINT point;
    ULONGLONG tickCount = GetTickCount64();
    ULONGLONG targetTime = tickCount +time; //2.5 seconds later
    product = (tickCount) % (unsigned long)(pow(256,sizeof(unsigned long))-1); //mod 4 bytes
    int chunkWriteCount = 1;
    unsigned long mod;
    if(n==NULL) mod = pow(256,sizeof(unsigned long))-1; //Can only get up to 256^unsigned long -2
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
            mod = pow(256,sizeof(unsigned long))-1;
            if(chunkWriteCount > chunks) break;
        }
        tickCount = GetTickCount64();
    }
    
    return 0;
}

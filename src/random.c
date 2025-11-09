#include "random.h"
#include <sysinfoapi.h>
#include <windows.h>
#include <winuser.h>
#include <math.h>
#include <stdint.h>
#include "sha.h"

int randomNumber(bignum bigIntArr, int chunks,bignum n,int generationMs){  //A chunk is 32 bits
    uint64_t product = 0; //uint32_t would mean the mod would never work and would output 0
    int x;
    int y;
    POINT point;
    ULONGLONG tickCount = GetTickCount64();
    //Random sources:

    //Hardware ticks
    LARGE_INTEGER perfCount;
    QueryPerformanceCounter(&perfCount); 
    product ^= perfCount.QuadPart;

    //Thread times
    FILETIME creationTime, exitTime, kernelModeTime, userModeTime;
    GetThreadTimes(GetCurrentThread(), &creationTime, &exitTime, &kernelModeTime, &userModeTime); 
    //creation time of process, exit time - undefined if no exit, amount of time executed in kernel mode, amount of time in user mode
    product ^= (*(uint64_t*)&kernelModeTime) ^ (*(uint64_t*)&userModeTime) ^ 
                (*(uint64_t*)&exitTime) ^ (*(uint64_t*)&creationTime);

    //Available memory
    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    GlobalMemoryStatusEx(&mem);
    product ^= mem.ullAvailPhys ^ mem.ullAvailVirtual;

    //PID and TID
    DWORD pid = GetCurrentProcessId();
    DWORD tid = GetCurrentThreadId();
    product ^= pid ^ tid;
    

    //CPU cycle counter
    unsigned __int64 cpuCycleCounter = __rdtsc(); 
    product ^= cpuCycleCounter;

    uint64_t stackVar; //get the address of a stack variable
    product ^= (uint64_t)&stackVar;

    ULONGLONG targetTime = tickCount + generationMs; 

    int chunkWriteCount = 1;
    uint32_t mod;
    if(n==NULL) mod = pow(256,sizeof(uint32_t))-1; //Can only get up to 256^uint32_t -2
    else mod = n[0];
    if(mod==0){
        perror("randomNumber: Modulus must be greater than 0");
        exit(1);
    }
    while(chunkWriteCount<=chunks){
        GetCursorPos(&point);
        x = point.x;
        y = point.y;
        product = (product ^ x ^ y)%mod;
        if(((LONGLONG) targetTime - (LONGLONG) tickCount) <= generationMs*((float)(chunks-chunkWriteCount)/(chunks+1))){ //+1 means chunks aren't written at start or end
            QueryPerformanceCounter(&perfCount); 
            product ^= perfCount.QuadPart;
            bigIntArr[chunkWriteCount-1] = (uint32_t) product;
            chunkWriteCount ++;
            mod = pow(256,sizeof(uint32_t))-1;
            if(chunkWriteCount > chunks) break;
        }
        tickCount = GetTickCount64();
    }
    //Spread out the randomness
    //We hash one 256 bit chunk and then move onto the next
    //Each time we pass the whole number (which includes previously hashed chunks) into sha256
    int nonHashedBytes = chunks*sizeof(uint32_t);
    int shaBytes = 256/8;
    int count = 0;
    printf("\nRandom number: ");
    for(int i=0;i<chunks;i++){
        printf("%x ",bigIntArr[i]);
    }
    while(nonHashedBytes>0){
        bignum hashed = sha256((uchar*)bigIntArr,chunks*sizeof(uint32_t)); 
        int copyLength = nonHashedBytes<shaBytes ? nonHashedBytes : shaBytes;
        memcpy(&bigIntArr[count*shaBytes/sizeof(uint32_t)],hashed,copyLength);
        free(hashed);
        nonHashedBytes -= shaBytes;
        count++;
    }   
    
   
    return 0;
}

#include "random.h"
#if _WIN32
    #include <sysinfoapi.h>
    #include <windows.h>
    #include <winuser.h>
#elif __linux__
    #include <time.h>
    #include <unistd.h>
    #include <sys/resource.h>
    #include <sys/sysinfo.h>
    #include <sys/syscall.h>
    #include <x86intrin.h>
    #include <X11/Xlib.h>
#endif 

#include <string.h>
#include <math.h>
#include <stdint.h>
#include "sha.h"

void randomNumber(bignum dest, int chunks,bignum n,int generationMs){  //A chunk is 32 bits
    uint64_t product = 0; //uint32_t would mean the mod would never work and would output 0
    
    uint64_t currTimeMs;
    #if _WIN32
        currTimeMs = (uint64_t) GetTickCount64();
        
        //Random sources:

        //High resolution timer
        LARGE_INTEGER perfCount;
        QueryPerformanceCounter(&perfCount); 
        product ^= perfCount.QuadPart;

        //Thread times
        FILETIME creationTime, exitTime, kernelModeTime, userModeTime;
        GetThreadTimes(GetCurrentThread(), &creationTime, &exitTime, &kernelModeTime, &userModeTime); 
        //Creation time of process, exit time - undefined if no exit, amount of time executed in kernel mode, amount of time in user mode
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

        char stackVar; //get the address of a stack variable
        product ^= (uint64_t)&stackVar;

        //Mouse position
        POINT point;
        GetCursorPos(&point);
        product = product ^ point.x ^ point.y;

    #elif __linux__
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC,&ts);
        currTimeMs = (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec/1000000;

        //Random sources:

        //High resolution timer
        product ^= ts.tv_nsec;

        //User and kernel times
        struct rusage ru;
        getrusage(RUSAGE_SELF, &ru);
        uint64_t userTime =
            ru.ru_utime.tv_sec * 1000000 +
            ru.ru_utime.tv_usec;

        uint64_t kernelTime =
            ru.ru_stime.tv_sec * 1000000 +
            ru.ru_stime.tv_usec;
        product ^= userTime ^ kernelTime;

        //Availible memory
        struct sysinfo info;
        sysinfo(&info);
        uint64_t freePhysical = info.freeram * info.mem_unit; //mem unit is the memory unit size in bytes
        uint64_t freeVirtual = info.freeswap * info.mem_unit;
        product ^= freePhysical ^ freeVirtual;

        //Process and thread ids
        pid_t pid = getpid();
        pid_t tid = syscall(SYS_gettid);
        product ^= pid ^ tid;
        
        //CPU cycles
        //Requires x86
        uint64_t cpuCyclesCounter = __rdtsc();
        product ^= cpuCyclesCounter;

        //Stack address
        char stackVar;
        product ^= (uint64_t)&stackVar;

        //Mouse position
        //System must be using X11 for graphics 
        Display *x11Display = XOpenDisplay(NULL);
        Window root = DefaultRootWindow(x11Display);
        int x, y;
        unsigned int mask;
        Window retRoot, retChild;
        XQueryPointer(x11Display, root, &retRoot, &retChild,
                    &x, &y, &x, &y, &mask);
        product ^= x ^ y;
    #endif
    
    uint64_t targetTime = currTimeMs + generationMs; 

    int chunkWriteCount = 1;
    uint32_t mod;
   
    while(chunkWriteCount<=chunks){
        //Use the mouse if the user has time to move it
        if(generationMs>=0){
            #if _WIN32
                GetCursorPos(&point);
                product = product ^ point.x ^ point.y;
            #elif __linux__
                XQueryPointer(x11Display, root, &retRoot, &retChild,
                            &x, &y, &x, &y, &mask);
                product ^= x ^ y;
            #endif
        }
        if((targetTime-currTimeMs) <= generationMs*((float)(chunks-chunkWriteCount)/(chunks+1))){ //+1 means chunks aren't written at start or end
            //Add the current time to the product so chunks are different
            #if _WIN32
                QueryPerformanceCounter(&perfCount); 
                product ^= perfCount.QuadPart;
            #elif __linux__
                clock_gettime(CLOCK_MONOTONIC,&ts);
                product ^= ts.tv_nsec;
            #endif
            
            dest[chunkWriteCount-1] = (uint32_t) product;
            chunkWriteCount ++;

            if(chunkWriteCount > chunks) break;
        }
        #if _WIN32
            currTimeMs = GetTickCount64();
        #elif __linux__
            clock_gettime(CLOCK_MONOTONIC,&ts);
            currTimeMs = (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec/1000000;
        #endif
    }
    #if __linux__
        XCloseDisplay(x11Display);
    #endif
    //Spread out the randomness
    //We hash one 256 bit chunk and then move onto the next
    //Each time we pass the whole number (which includes previously hashed chunks) into sha256
    int nonHashedBytes = chunks*sizeof(uint32_t);
    int shaBytes = 256/8;
    int count = 0;

    while(nonHashedBytes>0){
        bignum hashed = sha256((uchar*)dest,chunks*sizeof(uint32_t)); 
        int copyLength = nonHashedBytes<shaBytes ? nonHashedBytes : shaBytes;
        memcpy(&dest[count*shaBytes/sizeof(uint32_t)],hashed,copyLength);
        free(hashed);
        nonHashedBytes -= shaBytes;
        count++;
    }   


    if(n!=NULL){
        if(bigNumCmpLittle(n,chunks,0) == EQUAL){
            perror("randomNumber: Modulus must be greater than 0");
            exit(1);
        }
        
        //Make sure we are < n
        bigNumMod(dest,chunks,n,chunks,dest,chunks);
    }
}

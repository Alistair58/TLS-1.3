#ifndef GLOBALS_H
#define GLOBALS_H

#include <errno.h>
#include <stdlib.h>
#if _WIN32
    #include <winerror.h>
    #define ALLOC_EXIT_CODE ERROR_NO_MEMORY
#else 
    #define ALLOC_EXIT_CODE EXIT_FAILURE
#endif
#include <stdio.h>

#define allocError() char errorMsg[19+sizeof(__func__)]; \
    sprintf(errorMsg,"Alloc error in \"%s\"\n",__func__); \
    errno = ENOMEM; \
    perror(errorMsg); \
    exit(ALLOC_EXIT_CODE);
    

typedef unsigned char uchar;

#endif
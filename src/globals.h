#ifndef GLOBALS_H
#define GLOBALS_H

#include <errno.h>
#include <stdlib.h>
#include <winerror.h>

#define allocError() char errorMsg[19+sizeof(__func__)]; \
    sprintf(errorMsg,"\nAlloc error in \"%s\"",__func__); \
    errno = ENOMEM; \
    perror(errorMsg); \
    exit(ERROR_NOT_ENOUGH_MEMORY);

typedef unsigned char uchar;

#endif
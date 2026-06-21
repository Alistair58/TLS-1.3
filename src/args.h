#ifndef ARGS_H
#define ARGS_H

#include "stdint.h"

#define KEY_GEN 1 
#define CERTIF_GEN 2
#define CERTIF_SIGN 3
#define CONNECT 4 
#define DEALT_WITH 5 //E.g. -help or an invalid option


typedef struct Args{
    uint8_t option; 
    char *arg1;
    char *arg2;
    char *arg3;
} Args;  

/**
 * Parse the command line arguments for the server program into the Args structure.
 * If it is unsuccessful, or -help is the option, then it prints an output message. 
 * Caller must free all arg{n} in Args
 */
Args parseArgsServer(int argc,char **argv);

/**
 * Parse the command line arguments for the client program into the Args structure.
 * If it is unsuccessful, or -help is the option, then it prints an output message. 
 */
Args parseArgsClient(int argc,char **argv);

#endif 




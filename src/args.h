#ifndef ARGS_H
#define ARGS_H

#include "stdint.h"


typedef enum ArgOption{
    KEY_GEN = 1,
    CERTIF_GEN,
    CERTIF_SIGN,
    CONNECT,
    LISTEN,
    DEALT_WITH, //E.g. -help or an invalid option
    UNKNOWN
} ArgOption;


typedef struct Args{
    ArgOption option; 
    char *arg1;
    char *arg2;
    char *arg3;
} Args;  

/**
 * Parse the command line arguments for the server program into the Args structure.
 * If it is unsuccessful, or -help is the option, then it prints an output message. 
 */
Args parseArgsServer(int argc,char **argv);

/**
 * Parse the command line arguments for the client program into the Args structure.
 * If it is unsuccessful, or -help is the option, then it prints an output message. 
 */
Args parseArgsClient(int argc,char **argv);

#endif 




#include "args.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "globals.h"

static bool startsWith(char *str,char *prefix);
static char *parseQuotedString(char *str,bool assertEnd);
static Args parseCertifgen(Args res,char **argv);
static Args parseKeygen(Args res,char **argv);
static Args parseClientConnect(Args res,char **argv);
static Args parseCertifsign(Args res,char **argv);

#define UNKNOWN 6 //Internal

/**
 * Parse the command line arguments for the server program into the Args structure.
 * If it is unsuccessful, or -help is the option, then it prints an output message. 
 * Caller must free all arg{n} in Args
 */
Args parseArgsServer(int argc,char **argv){
    Args res = {.option = UNKNOWN};
    if(argc == 2){
        if(strcmp(argv[1],"-help")==0 || strcmp(argv[1],"--help")==0){
            printf(
            "Usage:\n"
            "   -keygen -privpath=\"{str}\" -pubpath=\"{str}\"\n"
            "       Generates a key pair and stores the private and public keys at the specified file paths.\n"
            "   -certifgen -subject=\"{str}\" -pubpath=\"{str}\"\n"
            "       Generates an unsigned X509 certificate for this subject.\n"
            "       pubpath is the path of the subject's public key.\n"
            "   -certifsign -certifpath=\"{str}\" -issuer=\"{str}\" -privpath=\"{str}\"\n"
            "       Signs an X509 certificate using the private key specified by the path and with the issuer's name on the certificate.\n"
            "       certifpath is the source file which will be overwritten with the signed version.\n"
            "   -connect\n"
            "       Listen to a socket and wait for a client.\n" 
            "       Once connected, prompts user for input, encrypts this and sends this to the client. Prints response from client. Loop repeats.\n"
            "   -help/--help"
            "       This."
            );
            res.option = DEALT_WITH;
        }
        else if(strcmp(argv[1],"-connect")==0){
            res.option = CONNECT;            
        }        
    }
    else if(argc == 4){
        //-keygen -privpath="{str}" -pubpath="{str}"
        if(strcmp(argv[1],"-keygen")==0 && startsWith(argv[2],"-privpath=") && startsWith(argv[3],"-pubpath=")){
           return parseKeygen(res,argv);
        }
        //-certifgen -subject="{str}" -pubpath="{str}"
        else if(strcmp(argv[1],"-certifgen")==0 && startsWith(argv[2],"-subject=") && startsWith(argv[3],"-pubpath=")){
            return parseCertifgen(res,argv);
        }
    }
    else if(argc == 5){
        //-certifsign -certifpath="{str} -issuer="{str}" -privpath="{str}"
        if(strcmp(argv[1],"-certifsign")==0 && startsWith(argv[2],"-certifpath=") && startsWith(argv[3],"-issuer=") && startsWith(argv[4],"-privpath=")){
            return parseCertifsign(res,argv);
        }
    }
    //Doing this means we don't have duplicate error branches
    if(res.option == UNKNOWN){
        printf("Invalid option. Run -help for info on valid options.\n");
        res.option = DEALT_WITH;
    }
    return res;
}

/**
 * Parse the command line arguments for the client program into the Args structure.
 * If it is unsuccessful, or -help is the option, then it prints an output message. 
 */
Args parseArgsClient(int argc,char **argv){
    Args res = {.option = UNKNOWN};
    if(argc == 2){
        if(strcmp(argv[1],"-help")==0 || strcmp(argv[1],"--help")==0){
            printf(
            "Usage:\n"
            "   -keygen -privpath=\"{str}\" -pubpath=\"{str}\"\n"
            "       Generates a key pair and stores the private and public keys at the specified file paths.\n"
            "   -certifgen -subject=\"{str}\" -pubpath=\"{str}\"\n"
            "       Generates an unsigned X509 certificate for this subject.\n"
            "       pubpath is the path of the subject's public key.\n"
            "   -connect -capubpath=\"{str}\"\n"
            "       Connect to a server using the public key of a CA found at capubpath.\n" 
            "       Once connected, prompts user for input, encrypts this and sends this to the server. Prints response from server. Loop repeats.\n"
            "   -help/--help"
            "       This."
            );
            res.option = DEALT_WITH;
        } 
    }
    else if(argc == 3){
        //-connect -capubpath="{str}"
        if(strcmp(argv[1],"-connect")==0 && startsWith(argv[2],"-capubpath=")){
            return parseClientConnect(res,argv);
        }
    }   
    else if(argc == 4){
        //-keygen -privpath="{str}" -pubpath="{str}"
        if(strcmp(argv[1],"-keygen")==0 && startsWith(argv[2],"-privpath=") && startsWith(argv[3],"-pubpath=")){
            return parseKeygen(res,argv);
        }
        //-certifgen -subject="{str}" -pubpath="{str}"
        else if(strcmp(argv[1],"-certifgen")==0 && startsWith(argv[2],"-subject=") && startsWith(argv[3],"-pubpath=")){
            return parseCertifgen(res,argv);
        }
    }

    //Doing this means we don't have duplicate error branches
    if(res.option == UNKNOWN){
        printf("Invalid option. Run -help for info on valid options.\n");
        res.option = DEALT_WITH;
    }
    return res;

}

static Args parseKeygen(Args res,char **argv){
    //-keygen -privpath="{str}" -pubpath="{str}"
    char *privPath = &argv[2][strlen("-privpath=")];
    if(privPath == NULL){
        printf("Invalid private key path. Run -help for info on valid commands.\n");
        res.option = DEALT_WITH;
        return res;
    }
    char *pubPath = &argv[3][strlen("-pubpath=")];
    if(pubPath == NULL){
        printf("Invalid public key path. Run -help for info on valid commands.\n");
        res.option = DEALT_WITH;
        return res;
    }
    res.option = KEY_GEN;
    res.arg1 = privPath;
    res.arg2 = pubPath;
    return res;
}

static Args parseClientConnect(Args res,char **argv){
    //-connect -capubpath="{str}"
    char *caPubPath = &argv[2][strlen("-capubpath=")];
    if(caPubPath == NULL){
        printf("Invalid CA public key path. Run -help for info on valid commands.\n");
        res.option = DEALT_WITH;
        return res;
    }
    res.option = CONNECT;
    res.arg1 = caPubPath;
    return res;
}


static Args parseCertifsign(Args res,char **argv){
    //-certifsign -certifpath="{str} -issuer="{str}" -privpath="{str}"
    char *certifPath = &argv[2][strlen("-certifpath=")];
    if(certifPath == NULL){
        printf("Invalid certificate path. Run -help for info on valid commands.\n");
        res.option = DEALT_WITH;
        return res;
    }
    char *issuer = &argv[3][strlen("-issuer=")];
    if(issuer == NULL){
        printf("Invalid issuer name. Run -help for info on valid commands.\n");
        res.option = DEALT_WITH;
        return res;
    }
    char *privPath = &argv[4][strlen("-privpath=")];
    if(privPath == NULL){
        printf("Invalid private key path. Run -help for info on valid commands.\n");
        res.option = DEALT_WITH;
        return res;
    }
    res.option = CERTIF_SIGN;
    res.arg1 = certifPath;
    res.arg2 = issuer;
    res.arg3 = privPath;
    return res;
}

static Args parseCertifgen(Args res,char **argv){
    //-certifgen -subject="{str}" -pubpath="{str}"
    char *subject = &argv[2][strlen("-subject=")];
    if(subject == NULL){
        printf("Invalid subject name. Run -help for info on valid commands.\n");
        res.option = DEALT_WITH;
        return res;
    }
    char *pubPath = &argv[3][strlen("-pubpath=")];
    if(pubPath == NULL){
        printf("Invalid public key path. Run -help for info on valid commands.\n");
        res.option = DEALT_WITH;
        return res;
    }
    res.option = CERTIF_GEN;
    res.arg1 = subject;
    res.arg2 = pubPath;
    return res;
}
/**
 * True if str starts with prefix. False otherwise. Does bound checks.
 * prefix and str should be null terminated
 */
static bool startsWith(char *str,char *prefix){
    return strlen(str)>=strlen(prefix) && strncmp(str,prefix,strlen(prefix))==0;     
}

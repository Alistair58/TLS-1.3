#include "../other/args.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "../other/globals.h"


static void parseCertifgen(Args *res,char **argv);
static void parseKeygen(Args *res,char **argv);
static void parseConnect(Args *res,char **argv);
static void parseCertifsign(Args *res,char **argv);
static void parseListen(Args *res,char **argv);
static void parseServerHelp(Args *res,char **argv);
static void parseClientHelp(Args *res,char **argv);
static bool checkOption(int argc,char **argv,int numParams,char **params);
static bool startsWith(char *str,char *prefix);
static bool checkEmpty(char *str,char *name);

#define KEYGEN_USAGE \
 "   -keygen -kppath=\"{str}\" -pubpath=\"{str}\"\n" \
 "       Generates a key pair and stores the key pair and public key at the specified file paths.\n"
char *keygenParams[] = {"-keygen","-kppath=","-pubpath="};

#define CERTIFGEN_USAGE \
 "   -certifgen -subject=\"{str}\" -pubpath=\"{str}\" -outpath=\"{str}\"\n" \
 "       Generates an unsigned X509 certificate for this subject.\n" \
 "       pubpath is the path of the subject's public key.\n" \
 "       outpath is where the generated certificate will be stored.\n"
char *certifgenParams[] = {"-certifgen","-subject=","-pubpath=","-outpath="};

#define CERTIFSIGN_USAGE \
 "   -certifsign -certifpath=\"{str}\" -issuer=\"{str}\" -kppath=\"{str}\"\n" \
 "       Signs an X509 certificate using the private key specified by the key pair path and with the issuer's name on the certificate.\n" \
 "       certifpath is the source file which will be overwritten with the signed version.\n"
char *certifsignParams[] = {"-certifsign","-certifpath=","-issuer=","-kppath="};

#define LISTEN_USAGE \
 "   -listen\n" \ 
 "       Listen to a socket and wait for a client.\n" \ 
 "       Once connected, prompts user for input, encrypts this and sends this to the client. Prints response from client. Loop repeats.\n"
char *listenParams[] = {"-listen"};

#define HELP_USAGE \
 "   -help\n" \
 "       This.\n"
char *helpParams[] = {"-help"};

#define CONNECT_USAGE \
 "   -connect -capubpath=\"{str}\"\n" \
 "       Connect to a server using the public key of a CA found at capubpath.\n" \
 "       Once connected, prompts user for input, encrypts this and sends this to the server. Prints response from server. Loop repeats.\n"
char *connectParams[] = {"-connect","-capubpath="};

//Represents a command line option
typedef struct Option{
    //Parameter names that a valid invokation needs
    char **params;
    int lenParams;
    //Function that puts the values of the parameters into an Args struct
    void (*parser)(Args*,char**);
} Option;

//String explaining the command line options for the server program
#define SERVER_USAGES \
    KEYGEN_USAGE \
    CERTIFGEN_USAGE \
    CERTIFSIGN_USAGE \
    LISTEN_USAGE \
    HELP_USAGE 

const Option serverOptions[] = 
    {
        {.params = keygenParams,.lenParams = arr_length(keygenParams),.parser = parseKeygen},
        {.params = certifgenParams,.lenParams = arr_length(certifgenParams),.parser = parseCertifgen},
        {.params = certifsignParams,.lenParams = arr_length(certifsignParams),.parser = parseCertifsign},
        {.params = listenParams,.lenParams = arr_length(listenParams),.parser = parseListen}, 
        {.params = helpParams,.lenParams = arr_length(helpParams),.parser = parseServerHelp}
    };

//String explaining the command line options for the client program
#define CLIENT_USAGES \
    KEYGEN_USAGE \
    CERTIFSIGN_USAGE \ 
    CONNECT_USAGE \
    HELP_USAGE

const Option clientOptions[] = 
    {
        {.params = keygenParams,.lenParams = arr_length(keygenParams),.parser = parseKeygen},
        {.params = certifsignParams,.lenParams = arr_length(certifsignParams),.parser = parseCertifsign},
        {.params = connectParams,.lenParams = arr_length(connectParams),.parser = parseConnect}, 
        {.params = helpParams,.lenParams = arr_length(helpParams),.parser = parseClientHelp}
    };

/**
 * Parse the command line arguments for the server program into the Args structure.
 * If it is unsuccessful, or -help is the option, then it prints an output message. 
 */
Args parseArgsServer(int argc,char **argv){
    Args res = {.option = UNKNOWN};
    for(int i=0;i<arr_length(serverOptions);i++){
        if(checkOption(argc,argv,serverOptions[i].lenParams,serverOptions[i].params)){
            (serverOptions[i].parser)(&res,argv);
            return res;
        }
    }
    printf("Invalid option. Run -help for info on valid options.\n");
    res.option = DEALT_WITH;
    return res;
}

/**
 * Parse the command line arguments for the client program into the Args structure.
 * If it is unsuccessful, or -help is the option, then it prints an output message. 
 */
Args parseArgsClient(int argc,char **argv){
    Args res = {.option = UNKNOWN};
    for(int i=0;i<arr_length(clientOptions);i++){
        if(checkOption(argc,argv,clientOptions[i].lenParams,clientOptions[i].params)){
            (clientOptions[i].parser)(&res,argv);
            return res;
        }
    }
    printf("Invalid option. Run -help for info on valid options.\n");
    res.option = DEALT_WITH;
    return res;
}

static void parseKeygen(Args *res,char **argv){
    //-keygen -kppath="{str}" -pubpath="{str}"
    char *kpPath = &argv[2][strlen("-kppath=")];
    char *pubPath = &argv[3][strlen("-pubpath=")];
    if(
        checkEmpty(kpPath,"kppath") ||
        checkEmpty(pubPath,"pubpath")
    ){
        res->option = DEALT_WITH;
        return;
    }
    res->option = KEY_GEN;
    res->arg1 = kpPath;
    res->arg2 = pubPath;
}

static void parseConnect(Args *res,char **argv){
    //-connect -capubpath="{str}"
    char *caPubPath = &argv[2][strlen("-capubpath=")];
    if(checkEmpty(caPubPath,"capubpath")){
        res->option = DEALT_WITH;
    }
    res->option = CONNECT;
    res->arg1 = caPubPath;
}


static void parseCertifsign(Args *res,char **argv){
    //-certifsign -certifpath="{str} -issuer="{str}" -kppath="{str}"
    char *certifPath = &argv[2][strlen("-certifpath=")];
    char *issuer = &argv[3][strlen("-issuer=")];
    char *kpPath = &argv[4][strlen("-kppath=")];
    if(
        checkEmpty(certifPath,"certifpath") || 
        checkEmpty(issuer,"issuer")         || 
        checkEmpty(kpPath,"kppath")
    ){
        res->option = DEALT_WITH;
        return;
    }
    res->option = CERTIF_SIGN;
    res->arg1 = certifPath;
    res->arg2 = issuer;
    res->arg3 = kpPath;
}

static void parseCertifgen(Args *res,char **argv){
    //-certifgen -subject="{str}" -pubpath="{str}" -outpath="{str}"
    char *subject = &argv[2][strlen("-subject=")];
    char *pubPath = &argv[3][strlen("-pubpath=")];
    char *outPath = &argv[4][strlen("-outpath=")];
    if(
        checkEmpty(subject,"subject") || 
        checkEmpty(pubPath,"pubpath") || 
        checkEmpty(outPath,"outpath")
    ){
        res->option = DEALT_WITH;
        return ;
    }
    res->option = CERTIF_GEN;
    res->arg1 = subject;
    res->arg2 = pubPath;
    res->arg3 = outPath;
}

static void parseServerHelp(Args *res,char **argv){
    printf(
        "Usage:\n"
        SERVER_USAGES
    );
    res->option = DEALT_WITH;
}

static void parseClientHelp(Args *res,char **argv){
    printf(
        "Usage:\n"
        CLIENT_USAGES
    );
    res->option = DEALT_WITH;
}


static void parseListen(Args *res,char **argv){
    res->option = LISTEN;
}


/**
 * True if argv has the structure of params. False otherwise.
 * The first string in params should be the name of the option.
 * The following strings should be the start of the option e.g. "-name="
 */
static bool checkOption(int argc,char **argv,int numParams,char **params){
    if(
        argc != numParams+1 || argc<2 ||
        //The first param is always the option name and must match exactly 
        strcmp(argv[1],params[0]) != 0
    ){
        return false;
    }
    for(int i=2;i<argc;i++){
        //Other options may have values and so only the start needs to match
        if(!startsWith(argv[i],params[i-1])){
            return false;
        }
    }
    return true;
}

/**
 * True if str starts with prefix. False otherwise. Does bound checks.
 * prefix and str should be null terminated
 */
static bool startsWith(char *str,char *prefix){
    return strlen(str)>=strlen(prefix) && strncmp(str,prefix,strlen(prefix))==0;     
}

/**
 * True if str starts with a null terminator. False otherwise.
 * If true, it will print an error message using the name of the parameter.
 * name should be the name of the parameter that str is the value for
 */
static bool checkEmpty(char *str,char *name){
    if(str[0] == '\0'){
        printf("Empty value for parameter: %s\n",name);
        return true;
    }
    else{
        return false;
    }
}
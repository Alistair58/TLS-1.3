#if _WIN32
    #include <winsock2.h>
#elif __linux__
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif
#include <stdint.h>
#include <string.h>
#include "../crypto/rsa.h"
#include "../crypto/params.h"
#include "../pki/x509.h"
#include "../crypto/x25519.h"
#include "../other/random.h"
#include "../other/common.h"
#include "../pki/keystore.h"
#include "../other/args.h"
#include "../other/globals.h"
#include "../other/der.h"

int startServer(sockaddr_in* addr,int *sock);
struct ClientHello waitForRequest(int sock,char *buffer, int lenBuff);
struct ServerHello generateServerHello(uint32_t *privateDHRandom);
uint32_t *generatePrivateECDH(uint32_t *keyExchange,uint32_t *privateDH);
int sendServerHello(int sock,struct ServerHello serverHello, char *buffer, int lenBuff);


serverHelloInfo = {

};

int main(int argc, char** argv){
    Args args = parseArgsServer(argc,argv);
    switch (args.option){
        case KEY_GEN:
            //TODO not hardcoded length
            RSAKeyPair kp = generateKeys(512);
            //-keygen -kppath="arg1" -pubpath="arg2"
            saveKeyPair(kp,args.arg1);
            savePublicKey(kp.publicKey,args.arg2);

            printf("Private key generated and saved at %s\nPublic key generated and saved at %s\n",args.arg1,args.arg2);
            freeRSAKeyPair(kp);
            break;
        case CERTIF_GEN:
            //-certifgen -subject="arg1" -pubpath="arg2" -outpath="arg3"
            RSAPublicKey subjectPk = readPublicKey(args.arg2);
            generateX509(subjectPk,args.arg1,args.arg3);
            freeRSAPublicKey(subjectPk);
            break;
        case CERTIF_SIGN:
            //-certifsign -certifpath="arg1" -issuer="arg2" -keypairpath="arg3"
            RSAKeyPair issuerKp = readKeyPair(args.arg3);
            signX509(issuerKp,args.arg2,args.arg1);

            printf("Certificate signed and saved at %s\n", args.arg1);
            freeRSAKeyPair(issuerKp);
            break;
        case LISTEN:
            // TODO add ip and port to parse args
            char* ip = "127.0.0.1";
            int port = 80;
            sockaddr_in server_addr;
            memset(&server_addr,0,sizeof(server_addr));
            server_addr.sin_family = AF_INET; //ipv4
            server_addr.sin_port = htons(port);
            server_addr.sin_addr.s_addr = inet_addr(ip);
            int server_sock = 0;
            char buffer[1024];

            uint32_t *privateDHRandom = calloc(8,sizeof(uint32_t));

            if(startServer(&server_addr,&server_sock) == 0){
                while(1){
                    struct sockaddr_in client_addr;
                    socklen_t client_addr_len;
                    int client_sock = accept(server_sock,(struct sockaddr*)&client_addr,&client_addr_len);
                    printf("%s","Client connected\n");

                    struct ClientHello clientHello = waitForRequest(client_sock,buffer,1024);
                    
                    struct ServerHello serverHello = generateServerHello(privateDHRandom); 
                    sendServerHello(client_sock,serverHello,buffer,1024);
                    uint32_t *privateECDHKey = generatePrivateECDH(clientHello.keyExchange,privateDHRandom);
                    
                    // TODO make not ABABA
                    gcmReceiveMessage(client_sock,buffer,1024,privateECDHKey);
                    printf("Received: %s\n",buffer);
                    memset(buffer,0,sizeof(buffer));

                    char input[256] = {0};
                    while(fgets(input, sizeof(input), stdin)){
                        // TODO document
                        if(input == "q") {
                            break;
                        }
                        gcmSendMessage(client_sock,buffer,sizeof(buffer),privateECDHKey,input,strlen(input));
                        memset(input,0,sizeof(input));

                        gcmReceiveMessage(client_sock,buffer,sizeof(buffer),privateECDHKey);
                        printf("Received: %s\n",buffer);
                        memset(buffer,0,sizeof(buffer));    
                    }
                    close(client_sock);
                    printf("%s","Client disconnected\n");
                    free(privateECDHKey);
                }
            }
            free(privateDHRandom);
            break;
        default:
            break;
    }
    return 0;
}
struct ClientHello waitForRequest(int sock,char *buffer, int lenBuff){
    struct ClientHello clientHello;
    memset(buffer,0,lenBuff); //Remove any rubbish from buffer
    recv(sock,buffer,lenBuff,0);
    char *temp = calloc(512,sizeof(char));
    int j=0,len=0,count=-1;
    for(int i=0;i<lenBuff;i++){
        if(j!=len && j%8==0){ //If a chunk <= 8 characters (32 bits) has been reached
            if(len>0){
                temp[len] = '\0';
            }
            switch(count){
                case 0:
                    clientHello.clientRandom = strtoul(temp,NULL,16);
                    break;
                case 1:
                    clientHello.cipherSuites[0][1] = (int)strtol(&temp[2],NULL,16);
                    temp[2] = '\0';
                    clientHello.cipherSuites[0][0] = (int)strtol(temp,NULL,16);
                    break;
                case 2:
                    clientHello.supportedGroups[0] = (int)strtol(temp,NULL,16);
                    break;
                case 3:
                    clientHello.signatureAlgorithms[0] = (int)strtol(temp,NULL,16);
                    break;
                case 4:
                    clientHello.keyExchange[7 - j/8] = strtoul(temp,NULL,16);
                    break;

            }
            
        }
        if(j==0){ //Has to be outside other statement for 1st iteration
            memcpy(temp,&buffer[i],2*sizeof(char)); //Read size of next chunk
            temp[2] = '\0';
            j = (int)strtol(temp,NULL,16);
            if(!j) break;
            len = j; //len remains constant while j decreases
            count++;
            i+=2;
        }
        temp[(len-j)%8] = buffer[i];
        j--;
    }
    printf("clientRandom %08x cipher suites %02x%02x supported groups %04x signature algorithms %04x client key exchange %u %u %u %u %u %u %u %u\n", //Length in characters before each chunk
    clientHello.clientRandom,
    clientHello.cipherSuites[0][0],clientHello.cipherSuites[0][1],
    clientHello.supportedGroups[0],clientHello.signatureAlgorithms[0],
    clientHello.keyExchange[0],clientHello.keyExchange[1],
    clientHello.keyExchange[2],clientHello.keyExchange[3],
    clientHello.keyExchange[4],clientHello.keyExchange[5],
    clientHello.keyExchange[6],clientHello.keyExchange[7]);
    return clientHello;
}


struct ServerHello generateServerHello(uint32_t *privateDHRandom,uchar *certifFname){
    struct ServerHello serverHello;
    int cipherSuite[2] =   {0x13,TLS_AES_128_GCM_SHA256};
    serverHello.curveGroup = x25519;
    serverHello.signatureAlgorithm  = rsa_pss_pss_sha256;
    uint32_t serverRandom;
    randomNumber(&serverRandom,1,NULL,0);
    randomNumber(privateDHRandom,8,curve25519Params.n,0);

    uint32_t *publicECDHKey = X25519(curve25519Params.G[0],privateDHRandom);
    
    serverHello.serverRandom = serverRandom;
    memcpy(&serverHello.cipherSuite,&cipherSuite,sizeof(cipherSuite));
    memcpy(&serverHello.keyExchange,publicECDHKey,8*sizeof(uint32_t));
    
    x509ToDER();
    free(serverRandom);free(publicECDHKey);
    return serverHello;
}

uint32_t *generatePrivateECDH(uint32_t *keyExchange,uint32_t *privateDH){
    // printf("Client public ECDHE: %u %u %u %u %u %u %u %u Private DH: %u %u %u %u %u %u %u %u \n",
    // keyExchange[0],keyExchange[1],keyExchange[2],keyExchange[3],
    // keyExchange[4],keyExchange[5],keyExchange[6],keyExchange[7],
    // privateDH[0],privateDH[1],privateDH[2],privateDH[3],
    // privateDH[4],privateDH[5],privateDH[6],privateDH[7]);
    uint32_t *privateECDHKey = X25519(keyExchange,privateDH);
    printf("Server Private ECDHE: %u %u %u %u %u %u %u %u\n",
    privateECDHKey[0],privateECDHKey[1],privateECDHKey[2],privateECDHKey[3],
    privateECDHKey[4],privateECDHKey[5],privateECDHKey[6],privateECDHKey[7]);
    return privateECDHKey;
}

int sendServerHello(int sock,struct ServerHello serverHello, char *buffer, int lenBuff){
    memset(buffer,0,lenBuff); //Remove any rubbish from buffer
    memcpy()
    String serverHello = derEncodeServerHello(serverHello);
    if(serverHello.)
    send(sock,buffer,strlen(buffer),0);
}

derEncodeStruct(
    array of offsets
    array of types
)


String derEncodeServerHello(struct ServerHello serverHello){
    String result;
    //Allocate a buffer that is sufficient in size
    //We will resize when we know the length
    const int lenDataBuff = 2048;
    result.data = (uchar*) malloc(lenDataBuff);
    if(!result.data){
        allocError();
    }
    int index = 0;

    //Certif sequence {
    result.data[index++] = DER_SEQUENCE;
    int certifSequenceLengthIndex = index;
    //reserve the index
    index += derEncodeInt(&result.data[index],lenDataBuff-index,0);
    int certifSequenceStart = index;
    
    //  tbsCertif {
    String tbsDER = asn1TBSToDER(asn1Certif.tbsCertif);
    memcpy(&result.data[index],tbsDER.data,tbsDER.lenData);
    index += tbsDER.lenData;
    free(tbsDER.data);
    //  } tbsCertif 

    //  Signature Algorithm {
    index += derEncodeInt(&result.data[index],lenDataBuff-index,asn1Certif.signatureAlgorithm);
    //  }


    //  Signature value and length {
    index += derEncodeBignum(&result.data[index],lenDataBuff-index,asn1Certif.signatureValue,asn1Certif.lenSignatureValue);
    //  }

    derEncodeInt(&result.data[certifSequenceLengthIndex],lenDataBuff-certifSequenceLengthIndex,index-certifSequenceStart);
    //} Certif sequence

    uchar *resizedResult = realloc(result.data,index);
    if(!resizedResult){
        allocError();
    }
    result.data = resizedResult;
    result.lenData = index;
    return result;
}

int startServer(sockaddr_in* addr,int *sock){
    // TODO tidy up - add closing of winsock and stuff
    #if _WIN32
        WSADATA wsa;
        int n;
        printf("Initialising Winsock...\n");
        if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
        {
            printf("Failed. Error Code : %d\n",WSAGetLastError());
            return 1;
        }
        printf("Initialised.\n");
    #endif
    *sock = socket(AF_INET,SOCK_STREAM,0); // IPv4, TCP
    
    #if _WIN32
        if(*sock == INVALID_SOCKET)
    #elif __linux__
        if(*sock < 0)        
    #endif     
    {
        perror("Could not get socket\n");
        exit(1);
    }
    printf("%s","TCP server socket created\n");
                
    // Assigns address to the socket
    if(bind(*sock, (struct sockaddr*)addr, sizeof(*addr))<0){
        perror("Could not bind socket\n");
        exit(1);
    }
    printf("Socket binded to port: %d\n",ntohs(addr->sin_port));
    printf("Listening\n");
    listen(*sock,5); //5 is the the maximum length to which the queue of pending connections can grow
    return 0;
}


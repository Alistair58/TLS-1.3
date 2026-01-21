#include <stdint.h>
#if _WIN32
    #include <winsock2.h>
#endif
#include <stdio.h>
#include <string.h>
#include "bigmaths.h"
#include "structs.h"
#include "sha.h"
#include "random.h"
#include "x25519.h"
#include "shared.h"

typedef unsigned char uchar;

int connectToServer(in_addr* addr, int* sock);
int sendClientHello(int sock,in_addr addr,char* buffer,int lenBuff,struct ClientHello clientHello);
struct ClientHello generateClientHello(uint32_t *privateDHRandom);
struct ServerHello waitForServerHello(int sock, char *buffer, int lenBuff);
uint32_t *generatePrivateECDH(uint32_t *keyExchange,uint32_t *privateDH);


//DONE

//TODO High-Level
//Add signatures
//Make messages more formal/conform to TLS-1.3
//Send everything in hex
//Add finished (a MAC over the handshake)

//TODO Low-Level
//Write private key stores
//Add them into x509 


int main(int argc, char** argv) {
    bignum result = sha256("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",56);
    printf("%08x%08x%08x%08x%08x%08x%08x%08x\n",result[0],result[1],result[2],result[3],result[4],result[5],result[6],result[7]);
    free(result);
    // int sock;
    // struct sockaddr_in addr;
    // char buffer[1024];
    // uint32_t *privateDHRandom = calloc(8,sizeof(uint32_t));
    // ClientHello clientHello = generateClientHello(privateDHRandom);
    // if(connectToServer(&addr,&sock)==0){
    //     sendClientHello(sock,addr,buffer,1024,clientHello);
    //     struct ServerHello serverHello = waitForServerHello(sock,buffer,1024);
    //     uint32_t *privateECDHKey = generatePrivateECDH(serverHello.keyExchange,privateDHRandom);

    //     char *testMessage = "Hello world!";
    //     gcmSendMessage(sock,buffer,1024,privateECDHKey,testMessage,12);
        
    //     close(sock);
    //     printf("Disconnected from server.\n");
    //     free(privateECDHKey);free(privateDHRandom);
    // }
    return 0;
}

int connectToServer(in_addr* addr, int* sock){
    //TODO make linux version
    #if _WIN32
        char* ip = "127.0.0.1";
        int port = 80;
        WSADATA wsa;
        int n;
        printf("Initialising Winsock...\n");
        if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
        {
            printf("Failed. Error Code : %d\n",WSAGetLastError());
            return 1;
        }
        printf("Initialised.\n");
        *sock = socket(AF_INET,SOCK_STREAM,0); //ipv4, tcp, IP protocol (0) - returns an int
        if(*sock == INVALID_SOCKET){
            perror("Could not get socket\n");
            exit(1);
        }
        printf("%s","TCP client socket created\n");
        
        memset(addr,0,sizeof(*addr));
        (*addr).sin_family = AF_INET; //ipv4
        (*addr).sin_port = port;
        (*addr).sin_addr.s_addr = inet_addr(ip);

        n = connect(*sock, (struct sockaddr*)addr, sizeof(*addr)); 
        if(n<0){
            perror("Could not connect\n");
            exit(1);
        }
        printf("%s","Connected to server\n");
    #endif
    return 0;
}

struct ClientHello generateClientHello(uint32_t *privateDHRandom){
    struct ClientHello clientHello;
    int cipherSuites[1][2] =   {{0x13,TLS_AES_128_GCM_SHA256}};
    int curveGroups[1] = {x25519};
    int signatureAlgorithms[1] = {rsa_pss_pss_sha256};
    uint32_t *clientRandom = calloc(1,sizeof(uint32_t));
    printf("Generating random number. Please move your mouse until generation is completed\n");
    randomNumber(clientRandom,1,NULL,500);
    randomNumber(privateDHRandom,8,curve25519Params.n,500);
    printf("Generation completed\n");
   // printf("Client random %u Client private DH Random: %u %u %u %u %u %u %u %u\n",clientRandom[0],privateDHRandom[0],privateDHRandom[1],privateDHRandom[2],privateDHRandom[3],
   // privateDHRandom[4],privateDHRandom[5],privateDHRandom[6],privateDHRandom[7]);
    uint32_t *ECDHKey  = X25519(curve25519Params.G[0],privateDHRandom);
    //printf("Client Public ECDHE: %u %u %u %u %u %u %u %u\n",ECDHKey[0],ECDHKey[1],ECDHKey[2],ECDHKey[3],ECDHKey[4],ECDHKey[5],ECDHKey[6],ECDHKey[7]);

    clientHello.clientRandom = clientRandom[0];
    memcpy(&clientHello.cipherSuites,&cipherSuites,sizeof(cipherSuites));
    memcpy(&clientHello.supportedGroups,&curveGroups ,sizeof(curveGroups));
    memcpy(&clientHello.signatureAlgorithms,&signatureAlgorithms,sizeof(signatureAlgorithms));
    memcpy(&clientHello.keyExchange,ECDHKey,8*sizeof(uint32_t));
    free(clientRandom);free(ECDHKey);
    return clientHello;
}

int sendClientHello(int sock,in_addr addr,char *buffer,int lenBuff,struct ClientHello clientHello){
    memset(buffer,0,lenBuff); //Remove any rubbish from buffer
    sprintf(buffer,"08%08x04%02x%02x04%04x04%04x40%08x%08x%08x%08x%08x%08x%08x%08x", //Length in characters before each chunk
    clientHello.clientRandom,
    clientHello.cipherSuites[0][0],clientHello.cipherSuites[0][1],
    clientHello.supportedGroups[0],clientHello.signatureAlgorithms[0],
    clientHello.keyExchange[0],clientHello.keyExchange[1],
    clientHello.keyExchange[2],clientHello.keyExchange[3],
    clientHello.keyExchange[4],clientHello.keyExchange[5],
    clientHello.keyExchange[6],clientHello.keyExchange[7]);
    printf("Sent clientRandom %08x cipher suites %02x%02x supported groups %04x signature algorithms %04x client key exchange %u %u %u %u %u %u %u %u\n", //Length in characters before each chunk
    clientHello.clientRandom,
    clientHello.cipherSuites[0][0],clientHello.cipherSuites[0][1],
    clientHello.supportedGroups[0],clientHello.signatureAlgorithms[0],
    clientHello.keyExchange[0],clientHello.keyExchange[1],
    clientHello.keyExchange[2],clientHello.keyExchange[3],
    clientHello.keyExchange[4],clientHello.keyExchange[5],
    clientHello.keyExchange[6],clientHello.keyExchange[7]);
    //TODO linux
    #if _WIN32
        send(sock,buffer,strlen(buffer),0);
    #endif
}

struct ServerHello waitForServerHello(int sock, char *buffer, int lenBuff){
    struct ServerHello serverHello;
    memset(buffer,0,lenBuff); //Remove any rubbish from buffer
    //TODO linux
    #if _WIN32
        recv(sock,buffer,lenBuff,0);
    #endif
    char *temp = calloc(512,sizeof(char));
    int j=0,len=0,count=-1;
    for(int i=0;i<lenBuff;i++){
        if(j!=len && j%8==0){ //If a chunk <= 8 characters (32 bits) has been reached
            if(len>0){
                temp[len] = '\0';
            }
            switch(count){
                case 0:
                    serverHello.serverRandom = strtoul(temp,NULL,16);
                    break;
                case 1:
                    serverHello.cipherSuite[1] = (int)strtol(&temp[2],NULL,16);
                    temp[2] = '\0';
                    serverHello.cipherSuite[0] = (int)strtol(temp,NULL,16);
                    break;
                case 2:
                    serverHello.curveGroup = (int)strtol(temp,NULL,16);
                    break;
                case 3:
                    serverHello.signatureAlgorithm = (int)strtol(temp,NULL,16);
                    break;
                case 4:
                    serverHello.keyExchange[7 - j/8] = strtoul(temp,NULL,16);
                    break;

            }
            
        }
        if(j==0){ //Has to be outside other statement for 1st iteration
            memcpy(temp,&buffer[i],2*sizeof(char)); //Read size of next chunk
            temp[2] = '\0';
            j = (int)strtol(temp,NULL,16); //j is the size of the next chunk
            if(!j) break;
            len = j; //len remains constant while j decreases
            count++;
            i+=2;
        }
        temp[(len-j)%8] = buffer[i];
        j--;
    }
    printf("Received serverRandom %08x cipher suite %02x%02x supported group %04x signature algorithm %04x Server key exchange %u %u %u %u %u %u %u %u\n", //Length in characters before each chunk
    serverHello.serverRandom,
    serverHello.cipherSuite[0],serverHello.cipherSuite[1],
    serverHello.curveGroup,serverHello.signatureAlgorithm,
    serverHello.keyExchange[0],serverHello.keyExchange[1],
    serverHello.keyExchange[2],serverHello.keyExchange[3],
    serverHello.keyExchange[4],serverHello.keyExchange[5],
    serverHello.keyExchange[6],serverHello.keyExchange[7]);
    return serverHello;
}

uint32_t *generatePrivateECDH(uint32_t *keyExchange,uint32_t *privateDH){
    // printf("Server public ECDHE: %u %u %u %u %u %u %u %u Private DH: %u %u %u %u %u %u %u %u \n",
    // keyExchange[0],keyExchange[1],keyExchange[2],keyExchange[3],
    // keyExchange[4],keyExchange[5],keyExchange[6],keyExchange[7],
    // privateDH[0],privateDH[1],privateDH[2],privateDH[3],
    // privateDH[4],privateDH[5],privateDH[6],privateDH[7]);
    uint32_t *privateECDHKey = X25519(keyExchange,privateDH);
    printf("Client Private ECDHE: %u %u %u %u %u %u %u %u\n",
    privateECDHKey[0],privateECDHKey[1],privateECDHKey[2],privateECDHKey[3],
    privateECDHKey[4],privateECDHKey[5],privateECDHKey[6],privateECDHKey[7]);
    return privateECDHKey;
}


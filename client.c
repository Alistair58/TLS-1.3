#include <stdio.h>
#include <stdbool.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "structs.h"
#include "random.h"
#include "x25519.h"
#include "aes.h"

int connectToServer(struct sockaddr_in* addr, int* sock);
int sendClientHello(int sock,struct sockaddr_in addr,char* buffer,int lenBuff,struct ClientHello clientHello);
struct ClientHello generateClientHello(unsigned long *privateDHRandom);
struct ServerHello waitForServerHello(int sock, char *buffer, int lenBuff);
unsigned long *generatePrivateECDH(unsigned long *keyExchange,unsigned long *privateDH);
//gcc client.c -o client.exe -g -l ws2_32


int main(int argc, char** argv) {
    int sock;
    struct sockaddr_in addr;
    char buffer[1024];
    // byte c[] = {2,1,1,3}; 
    // byte d[] = {0x0E,0x09,0x0D,0x0B};
    // byte e[] = {0xf2, 0x0a, 0x22, 0x5c};
    // byte result[] = {0,0,0,0};
    // vectorModMult(c,e,result);
    // printf("Result %x %x %x %x",result[0],result[1],result[2],result[3]);
    byte result1 = rCon(1);
    printf("Results %x",result1);
    // unsigned long keyArr[] = {1,2,3,4,5,6,7,8};
    // unsigned long *key = createBigNum(keyArr,8);
    // unsigned long dataArr[] = {0,0,0,0,0,0,10,20};
    // unsigned long *data = createBigNum(dataArr,8);
    // unsigned long *dest = calloc(8,sizeof(unsigned long));
    // printBigNum("Data",data,8);
    // aesEncrypt(key,data,dest);
    // printBigNum("Encrypted",dest,8);
    // aesDecrypt(key,dest,dest);
    // printBigNum("Decrypted",dest,8);
    // free(key);free(data);free(dest);
    /*unsigned long *privateDHRandom = calloc(8,sizeof(unsigned long));
    struct ClientHello clientHello = generateClientHello(privateDHRandom);
    if(connectToServer(&addr,&sock)==0){
        sendClientHello(sock,addr,buffer,1024,clientHello);
        struct ServerHello serverHello = waitForServerHello(sock,buffer,1024);
        unsigned long *privateECDHKey = generatePrivateECDH(serverHello.keyExchange,privateDHRandom);

        //AES IN GCM
        
        close(sock);
        printf("\nDisconnected from server.");
        free(privateECDHKey);free(privateDHRandom);
    }*/
    return 0;
}

int connectToServer(struct sockaddr_in * addr, int* sock){
    char* ip = "127.0.0.1";
    int port = 80;
    WSADATA wsa;
    int n;
    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
    {
        printf("\nFailed. Error Code : %d",WSAGetLastError());
        return 1;
    }
    printf("\nInitialised.");
    *sock = socket(AF_INET,SOCK_STREAM,0); //ipv4, tcp, IP protocol (0) - returns an int
    if(*sock == INVALID_SOCKET){
        perror("\nCould not get socket");
        exit(1);
    }
    printf("%s","\nTCP client socket created");
    
    memset(addr,0,sizeof(*addr));
    (*addr).sin_family = AF_INET; //ipv4
    (*addr).sin_port = port;
    (*addr).sin_addr.s_addr = inet_addr(ip);

    n = connect(*sock, (struct sockaddr*)addr, sizeof(*addr)); 
    if(n<0){
        perror("\nCould not connect");
        exit(1);
    }
    printf("%s","\nConnected to server");
    return 0;
}

struct ClientHello generateClientHello(unsigned long *privateDHRandom){
    struct ClientHello clientHello;
    int cipherSuites[1][2] =   {{0x13,TLS_AES_128_GCM_SHA256}};
    int curveGroups[1] = {x25519};
    int signatureAlgorithms[1] = {rsa_pss_pss_sha256};
    unsigned long *clientRandom = calloc(1,sizeof(unsigned long));
    printf("\nGenerating random number. Please move your mouse until generation is completed");
    randomNumber(clientRandom,1,NULL);
    randomNumber(privateDHRandom,8,curve25519Params.n);
    printf("\nGeneration completed");
    printf("\nClient random %lu Client private DH Random: %lu %lu %lu %lu %lu %lu %lu %lu",clientRandom[0],privateDHRandom[0],privateDHRandom[1],privateDHRandom[2],privateDHRandom[3],
    privateDHRandom[4],privateDHRandom[5],privateDHRandom[6],privateDHRandom[7]);
    unsigned long *ECDHKey  = X25519(curve25519Params.G[0],privateDHRandom);
    printf("\nClient Public ECDHE: %lu %lu %lu %lu %lu %lu %lu %lu",ECDHKey[0],ECDHKey[1],ECDHKey[2],ECDHKey[3],ECDHKey[4],ECDHKey[5],ECDHKey[6],ECDHKey[7]);

    clientHello.clientRandom = clientRandom[0];
    memcpy(&clientHello.cipherSuites,&cipherSuites,sizeof(cipherSuites));
    memcpy(&clientHello.supportedGroups,&curveGroups ,sizeof(curveGroups));
    memcpy(&clientHello.signatureAlgorithms,&signatureAlgorithms,sizeof(signatureAlgorithms));
    memcpy(&clientHello.keyExchange,ECDHKey,8*sizeof(unsigned long));
    free(clientRandom);free(ECDHKey);
    return clientHello;
}

int sendClientHello(int sock,struct sockaddr_in addr,char *buffer,int lenBuff,struct ClientHello clientHello){
    memset(buffer,0,lenBuff); //Remove any rubbish from buffer
    sprintf(buffer,"08%08x04%02x%02x04%04x04%04x40%08x%08x%08x%08x%08x%08x%08x%08x", //Length in characters before each chunk
    clientHello.clientRandom,
    clientHello.cipherSuites[0][0],clientHello.cipherSuites[0][1],
    clientHello.supportedGroups[0],clientHello.signatureAlgorithms[0],
    clientHello.keyExchange[0],clientHello.keyExchange[1],
    clientHello.keyExchange[2],clientHello.keyExchange[3],
    clientHello.keyExchange[4],clientHello.keyExchange[5],
    clientHello.keyExchange[6],clientHello.keyExchange[7]);
    printf("\nSent clientRandom %08x cipher suites %02x%02x supported groups %04x signature algorithms %04x key exchange %lu %lu %lu %lu %lu %lu %lu %lu", //Length in characters before each chunk
    clientHello.clientRandom,
    clientHello.cipherSuites[0][0],clientHello.cipherSuites[0][1],
    clientHello.supportedGroups[0],clientHello.signatureAlgorithms[0],
    clientHello.keyExchange[0],clientHello.keyExchange[1],
    clientHello.keyExchange[2],clientHello.keyExchange[3],
    clientHello.keyExchange[4],clientHello.keyExchange[5],
    clientHello.keyExchange[6],clientHello.keyExchange[7]);
    send(sock,buffer,strlen(buffer),0);
}

struct ServerHello waitForServerHello(int sock, char *buffer, int lenBuff){
    struct ServerHello serverHello;
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
            j = (int)strtol(temp,NULL,16);
            if(!j) break;
            len = j; //len remains constant while j decreases
            count++;
            i+=2;
        }
        temp[(len-j)%8] = buffer[i];
        j--;
    }
    printf("\nReceived serverRandom %08x cipher suite %02x%02x supported group %04x signature algorithm %04x key exchange %lu %lu %lu %lu %lu %lu %lu %lu", //Length in characters before each chunk
    serverHello.serverRandom,
    serverHello.cipherSuite[0],serverHello.cipherSuite[1],
    serverHello.curveGroup,serverHello.signatureAlgorithm,
    serverHello.keyExchange[0],serverHello.keyExchange[1],
    serverHello.keyExchange[2],serverHello.keyExchange[3],
    serverHello.keyExchange[4],serverHello.keyExchange[5],
    serverHello.keyExchange[6],serverHello.keyExchange[7]);
    return serverHello;
}

unsigned long *generatePrivateECDH(unsigned long *keyExchange,unsigned long *privateDH){
    printf("\nServer public ECDHE: %lu %lu %lu %lu %lu %lu %lu %lu Private DH: %lu %lu %lu %lu %lu %lu %lu %lu ",
    keyExchange[0],keyExchange[1],keyExchange[2],keyExchange[3],
    keyExchange[4],keyExchange[5],keyExchange[6],keyExchange[7],
    privateDH[0],privateDH[1],privateDH[2],privateDH[3],
    privateDH[4],privateDH[5],privateDH[6],privateDH[7]);
    unsigned long *privateECDHKey = X25519(keyExchange,privateDH);
    printf("\nClient Private ECDHE: %lu %lu %lu %lu %lu %lu %lu %lu",
    privateECDHKey[0],privateECDHKey[1],privateECDHKey[2],privateECDHKey[3],
    privateECDHKey[4],privateECDHKey[5],privateECDHKey[6],privateECDHKey[7]);
    return privateECDHKey;
}
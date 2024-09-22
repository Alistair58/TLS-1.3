#include <stdio.h>
#include <stdbool.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <winuser.h>
#include <math.h>
#include "structs.h"
#include "ciphers.h"

int connectToServer(struct sockaddr_in* addr, int* sock);
int sendClientHello(int sock,struct sockaddr_in addr,char* buffer);
//gcc client.c -o client.exe -g -l ws2_32

/*
USEFUL
memset(&buffer,0,sizeof(buffer)); //Remove any rubbish from buffer
strcpy(buffer,"Client Test Message");
send(sock,buffer,strlen(buffer),0);

memset(&buffer,0,sizeof(buffer)); 
recv(sock,buffer,sizeof(buffer),0);
printf("\nClient received: %s",buffer);
*/

int main(int argc, char** argv) {
    int sock;
    struct sockaddr_in addr;
    char buffer[1024];
    sendClientHello(-1,addr,buffer);
    /*if(connectToServer(&addr,&sock)==0){
        sendClientHello(sock,addr,buffer);
        close(sock);
        printf("\nDisconnected from server.");
        return 1;
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

int sendClientHello(int sock, struct sockaddr_in addr, char* buffer){
    int cipherSuites[2] =   {0x13,TLS_AES_128_GCM_SHA256};
    int curveGroups[1] = {secp256r};
    int signatureAlgorithms[1] = {rsa_pss_pss_sha256};
    //unsigned long dhKey = ECDH(secp256r);
    unsigned long *clientRandom = calloc(1,sizeof(unsigned long));
    unsigned long *privateDH = calloc(8,sizeof(unsigned long));
    unsigned long *ECDHKey = calloc(8,sizeof(unsigned long));
    printf("\nGenerating random number. Please move your mouse until generation is completed");
    randomNumber(clientRandom,1,NULL);
    printf("\n new one");
    randomNumber(privateDH,8,curve25519Params.n);
    printBigNum("Private random number ",privateDH,8);
    printf("\nGeneration completed");

    ECDHKey = X25519(curve25519Params.G[0],privateDH);
    printf("ECDHKey: %lu %lu %lu %lu %lu %lu %lu %lu",ECDHKey[0],ECDHKey[1],ECDHKey[2],ECDHKey[3],ECDHKey[4],ECDHKey[5],ECDHKey[6],ECDHKey[7]);

    /*struct ClientHello clientHello;
    clientHello.clientRandom = clientRandom[0];
    memcpy(clientHello.cipherSuites,cipherSuites,sizeof(cipherSuites));
    
    printf("\nClient random %lu",clientRandom[0]);*/
    return 0;
      

}
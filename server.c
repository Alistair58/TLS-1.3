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
int startServer(struct sockaddr_in * addr,int* sock);
//gcc server.c -o server.exe -l ws2_32

int main(int argc, char** argv) {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    char buffer[1024];
    
   
    if(startServer(&server_addr,&server_sock) == 0){
        while(1){
            addr_size = sizeof(client_addr);
            client_sock = accept(server_sock,(struct sockaddr*)&client_addr,&addr_size);
            printf("%s","\nClient connected");

            struct ClientHello clientHello = waitForRequest(client_sock,buffer,1024);
            
            //memset(&buffer,0,sizeof(buffer));
            //strcpy(buffer,"Server Test Message");
            //send(client_sock,buffer,strlen(buffer),0);

            close(client_sock);
            printf("%s","\nClient disconnected");
        }
    }
    
    return 0;

   
}
struct ClientHello waitForRequest(int sock,char *buffer, int lenBuff){
    struct ClientHello clientHello;
    memset(&buffer,0,sizeof(buffer)); //Remove any rubbish from buffer
    recv(sock,buffer,sizeof(buffer),0);
    printf("\nServer received: %s",buffer);
    char *temp = calloc(512,sizeof(char));
    int j=0,len=0,count=-1;
    for(int i=0;i<lenBuff;i++){
        if(j!=len && j%8==0){
            if(len>0){
                temp[len%8] = '\0';
            }
            switch(count){
                case 0:
                    clientHello.clientRandom = strtoul(temp,NULL,16);
                    break;
                case 1:
                    clientHello.cipherSuites[0][1] = strtoi(&temp[2],NULL,16);
                    temp[2] = '\0';
                    clientHello.cipherSuites[0][0] = strtoi(temp,NULL,16);
                    break;
                case 2:
                    clientHello.supportedGroups[0] = strtoi(temp,NULL,16);
                    break;
                case 3:
                    clientHello.signatureAlgorithms[0] = strtoi(temp,NULL,16);
                    break;
                case 4:
                    clientHello.keyExchange[7 - j/8] = strtoul(temp,NULL,16);
                    break;

            }
            if(j==0){
                memcpy(temp,&buffer[i],2*sizeof(char)); //Read size of next chunk
                temp[2] = '\0';
                j = strtoi(temp,NULL,16);
                if(!j) break;
                len = j; //len remains constant while j decreases
                count++;
            }
            
        }
        else{
            temp[(len-j)%8] = buffer[i];
            j--;
        }
    }
    printf("08%08x04%02x%02x04%04x04%04x40%08x%08x%08x%08x%08x%08x%08x%08x", //Length in characters before each chunk
    clientHello.clientRandom,
    clientHello.cipherSuites[0][0],clientHello.cipherSuites[0][1],
    clientHello.supportedGroups[0],clientHello.signatureAlgorithms[0],
    clientHello.keyExchange[0],clientHello.keyExchange[1],
    clientHello.keyExchange[2],clientHello.keyExchange[3],
    clientHello.keyExchange[4],clientHello.keyExchange[5],
    clientHello.keyExchange[6],clientHello.keyExchange[7]);
}

int startServer(struct sockaddr_in * addr, int* sock){
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
    printf("%s","\nTCP server socket created");
    
    memset(addr,0,sizeof(*addr));
    (*addr).sin_family = AF_INET; //ipv4
    (*addr).sin_port = port;
    (*addr).sin_addr.s_addr = inet_addr(ip);
    
    n = bind(*sock, (struct sockaddr*)addr, sizeof(*addr)); //assigns address to the socket
    if(n<0){
        perror("\nCould not bind socket");
        exit(1);
    }
    printf("%s%d","\nSocket binded to port: ",port);
    printf("\nListening");
    listen(*sock,5); //5 is the the maximum length to which the queue of pending connections can grow
    return 0;
}


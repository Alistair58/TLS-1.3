#include <stdio.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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

            memset(&buffer,0,sizeof(buffer)); //Remove any rubbish from buffer
            recv(client_sock,buffer,sizeof(buffer),0);
            printf("\nServer received: %s",buffer);
            
            memset(&buffer,0,sizeof(buffer));
            strcpy(buffer,"Server Test Message");
            send(client_sock,buffer,strlen(buffer),0);

            close(client_sock);
            printf("%s","\nClient disconnected");
        }
    }
    
    return 0;

   
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


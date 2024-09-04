#include <stdio.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//gcc server.c -o server.exe -l ws2_32

int main(int argc, char** argv) {
    char* ip = "127.0.0.1";
    int port = 80;
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    char buffer[1024];
    int n;
    WSADATA wsa;
    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
    {
        printf("Failed. Error Code : %d",WSAGetLastError());
        return 1;
    }

    printf("Initialised.\n");
    server_sock = socket(AF_INET,SOCK_STREAM,0); //ipv4, tcp, IP protocol (0) - returns an int
    if(server_sock == INVALID_SOCKET){
        perror("Could not get socket");
        exit(1);
    }
    printf("%s","TCP server socket created\n");
    
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET; //ipv4
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    
    n = bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)); //assigns address to the socket
    if(n<0){
        perror("Could not bind socket");
        exit(1);
    }
    printf("%s%d","Socket binded to port: ",port);
    
    listen(server_sock,5); //5 is the the maximum length to which the queue of pending connections can grow
    
    while(1){
        addr_size = sizeof(client_addr);
        client_sock = accept(server_sock,(struct sockaddr*)&client_addr,&addr_size);
        printf("%s","Client connected");
    }
    return 0;
}


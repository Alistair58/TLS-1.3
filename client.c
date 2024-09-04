#include <stdio.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//gcc client.c -o client.exe -l ws2_32

int main(int argc, char** argv) {
    char* ip = "127.0.0.1";
    int port = 80;
    int sock;
    struct sockaddr_in addr;
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
    sock = socket(AF_INET,SOCK_STREAM,0); //ipv4, tcp, IP protocol (0) - returns an int
    if(sock == INVALID_SOCKET){
        perror("Could not get socket");
        exit(1);
    }
    printf("%s","TCP client socket created\n");
    
    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET; //ipv4
    addr.sin_port = port;
    addr.sin_addr.s_addr = inet_addr(ip);
    
    n = connect(sock, (struct sockaddr*)&addr, sizeof(addr)); 
    if(n<0){
        perror("Could not connect");
        exit(1);
    }
    printf("%s%d","Connected to port: ",port);
    return 0;
}


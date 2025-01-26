#include "shared.h"

typedef unsigned char uchar;

int startServer(struct sockaddr_in * addr,int* sock);
struct ClientHello waitForRequest(int sock,char *buffer, int lenBuff);
struct ServerHello generateServerHello(unsigned long *privateDHRandom);
unsigned long *generatePrivateECDH(unsigned long *keyExchange,unsigned long *privateDH);
int sendServerHello(int sock,struct ServerHello serverHello, char *buffer, int lenBuff);

//gcc server.c -o server.exe -l ws2_32

int main(int argc, char** argv) {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    unsigned long *privateDHRandom = calloc(8,sizeof(unsigned long));
    char buffer[1024];
    
   
    if(startServer(&server_addr,&server_sock) == 0){
        while(1){
            addr_size = sizeof(client_addr);
            client_sock = accept(server_sock,(struct sockaddr*)&client_addr,&addr_size);
            printf("%s","\nClient connected");

            struct ClientHello clientHello = waitForRequest(client_sock,buffer,1024);
            
            struct ServerHello serverHello = generateServerHello(privateDHRandom); 
            sendServerHello(client_sock,serverHello,buffer,1024);
            unsigned long *privateECDHKey = generatePrivateECDH(clientHello.keyExchange,privateDHRandom);

            gcmReceiveMessage(client_sock,buffer,1024,privateECDHKey);
            close(client_sock);
            printf("%s","\nClient disconnected");
            free(privateECDHKey);
            
        }
    }
    free(privateDHRandom);
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
    printf("\nclientRandom %08x cipher suites %02x%02x supported groups %04x signature algorithms %04x client key exchange %lu %lu %lu %lu %lu %lu %lu %lu", //Length in characters before each chunk
    clientHello.clientRandom,
    clientHello.cipherSuites[0][0],clientHello.cipherSuites[0][1],
    clientHello.supportedGroups[0],clientHello.signatureAlgorithms[0],
    clientHello.keyExchange[0],clientHello.keyExchange[1],
    clientHello.keyExchange[2],clientHello.keyExchange[3],
    clientHello.keyExchange[4],clientHello.keyExchange[5],
    clientHello.keyExchange[6],clientHello.keyExchange[7]);
    return clientHello;
}


struct ServerHello generateServerHello(unsigned long *privateDHRandom){
    struct ServerHello serverHello;
    int cipherSuite[2] =   {0x13,TLS_AES_128_GCM_SHA256};
    serverHello.curveGroup = x25519;
    serverHello.signatureAlgorithm  = rsa_pss_pss_sha256;
    unsigned long *serverRandom = calloc(1,sizeof(unsigned long));
    printf("\nGenerating random number. Please move your mouse until generation is completed");
    randomNumber(serverRandom,1,NULL);
    randomNumber(privateDHRandom,8,curve25519Params.n);
    printf("\nGeneration completed");
    // printf("\nServer random %lu Server Private DH Random: %lu %lu %lu %lu %lu %lu %lu %lu ",serverRandom[0],privateDHRandom[0],privateDHRandom[1],privateDHRandom[2],privateDHRandom[3],
    // privateDHRandom[4],privateDHRandom[5],privateDHRandom[6],privateDHRandom[7]);
    unsigned long *publicECDHKey = X25519(curve25519Params.G[0],privateDHRandom);
    // printf("\nServer Public ECDHE: %lu %lu %lu %lu %lu %lu %lu %lu",
    // publicECDHKey[0],publicECDHKey[1],publicECDHKey[2],publicECDHKey[3],
    // publicECDHKey[4],publicECDHKey[5],publicECDHKey[6],publicECDHKey[7]);

    
    serverHello.serverRandom = serverRandom[0];
    memcpy(&serverHello.cipherSuite,&cipherSuite,sizeof(cipherSuite));
    memcpy(&serverHello.keyExchange,publicECDHKey,8*sizeof(unsigned long));
    free(serverRandom);free(publicECDHKey);
    return serverHello;
}

unsigned long *generatePrivateECDH(unsigned long *keyExchange,unsigned long *privateDH){
    // printf("\nClient public ECDHE: %lu %lu %lu %lu %lu %lu %lu %lu Private DH: %lu %lu %lu %lu %lu %lu %lu %lu ",
    // keyExchange[0],keyExchange[1],keyExchange[2],keyExchange[3],
    // keyExchange[4],keyExchange[5],keyExchange[6],keyExchange[7],
    // privateDH[0],privateDH[1],privateDH[2],privateDH[3],
    // privateDH[4],privateDH[5],privateDH[6],privateDH[7]);
    unsigned long *privateECDHKey = X25519(keyExchange,privateDH);
    printf("\nServer Private ECDHE: %lu %lu %lu %lu %lu %lu %lu %lu",
    privateECDHKey[0],privateECDHKey[1],privateECDHKey[2],privateECDHKey[3],
    privateECDHKey[4],privateECDHKey[5],privateECDHKey[6],privateECDHKey[7]);
    return privateECDHKey;
}

int sendServerHello(int sock,struct ServerHello serverHello, char *buffer, int lenBuff){
    memset(buffer,0,lenBuff); //Remove any rubbish from buffer
    sprintf(buffer,"08%08x04%02x%02x04%04x04%04x40%08x%08x%08x%08x%08x%08x%08x%08x", //Length in characters before each chunk
    serverHello.serverRandom,
    serverHello.cipherSuite[0],serverHello.cipherSuite[1],
    serverHello.curveGroup,serverHello.signatureAlgorithm,
    serverHello.keyExchange[0],serverHello.keyExchange[1],
    serverHello.keyExchange[2],serverHello.keyExchange[3],
    serverHello.keyExchange[4],serverHello.keyExchange[5],
    serverHello.keyExchange[6],serverHello.keyExchange[7]);
    printf("\nSent serverRandom %08x cipher suite %02x%02x supported group %04x signature algorithm %04x server key exchange %lu %lu %lu %lu %lu %lu %lu %lu", //Length in characters before each chunk
    serverHello.serverRandom,
    serverHello.cipherSuite[0],serverHello.cipherSuite[1],
    serverHello.curveGroup,serverHello.signatureAlgorithm,
    serverHello.keyExchange[0],serverHello.keyExchange[1],
    serverHello.keyExchange[2],serverHello.keyExchange[3],
    serverHello.keyExchange[4],serverHello.keyExchange[5],
    serverHello.keyExchange[6],serverHello.keyExchange[7]);
    send(sock,buffer,strlen(buffer),0);
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


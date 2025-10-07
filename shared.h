#include <stdio.h>
#include <stdbool.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <winuser.h>
#include <math.h>
#include <errno.h>
#define allocError() char errorMsg[19+sizeof(__func__)]; \
    sprintf(errorMsg,"\nAlloc error in \"%s\"",__func__); \
    errno = ENOMEM; \
    perror(errorMsg); \
    exit(ERROR_NOT_ENOUGH_MEMORY);
#include "structs.h"
#include "x25519.h"
#include "aes.h"
#include "random.h"
#include "gcm.h" 
#include "rsa.h"
#include "x509.h"
#include "sha.h"



void ecbSendMessage(int sock,uchar *buffer,int lenBuff,uint32_t *key,char *msg,int lenMsg);
void ecbReceiveMessage(int sock,char *buffer, int lenBuff, uint32_t *key);
void gcmSendMessage(int sock,uchar *buff,int lenBuff,uint32_t *key,char *msg,int lenMsg);
void gcmReceiveMessage(int sock,char *buffer, int lenBuff, uint32_t *key);

typedef unsigned char uchar;


void ecbSendMessage(int sock,uchar *buffer,int lenBuff,uint32_t *key,char *msg,int lenMsg){
    memset(buffer,0,lenBuff);
    if(lenBuff < lenMsg){
        perror("\nBuffer is too small for message");
        exit(1);
    }
    for(int i=0;i<ceil((float)lenMsg/256);i++){
        uint32_t block[8] = {0};
        for(int j=0;j<32;j++){
            if (i*32 + j <lenMsg) block[j>>2] |= msg[i*32 + j] << ((j&3)*8); //append message to block
        }
        aesEncrypt(key,block,block);
        memcpy(&buffer[i*32],block,32); //32 byte block
    }
    send(sock,buffer,lenBuff,0); 
    buffer[lenBuff-1] = '\0';
    printf("\nSent encrypted: ");
    for(int i=0;i<lenBuff;i++){
        printf("%c",buffer[i]); //if you print the string it stops at a 0
    }
}

void ecbReceiveMessage(int sock,char *buffer,int lenBuff,uint32_t *key){
    memset(buffer,0,lenBuff); //Remove any rubbish from buffer
    if(!recv(sock,buffer,lenBuff,0) || buffer[0]==0){
        printf("\nNo message received from the client");
    }
    else{
        buffer[lenBuff-1] = '\0';
        printf("\nBuffer received %s",buffer);
        for(int i=0;i<floor((float)lenBuff/256);i++){
            uint32_t block[8] = {0};
            for(int j=0;j<32;j++){
                block[j>>2] |= ((uchar)buffer[i*32 + j]) << ((j&3)*8); //append message to block - if it is signed it causes a big mess
            }
            aesDecrypt(key,block,block);
            memcpy(&buffer[i*32],block,32); //32 byte block
        }
        buffer[lenBuff-1] = '\0';
    }
}

void gcmSendMessage(int sock,uchar *buff,int lenBuff,uint32_t *key,char *msg,int lenMsg){
    memset(buff,0,lenBuff);
    gcmResult result;
    result.iv = 0;
    result.tag = 0;
    int numBlocks = ceil((float)lenMsg/32);
    //3 hex digits for length of the messgae (4096 max length)
    //followed by 32*numBlocks chars of encrypted message
    //Followed by 32 hex characters of iv
    //Followed by 32 hex characters of tag
    //Null terminator
    int lenToSend = 32*numBlocks+132;
    if(lenBuff < lenToSend){
        perror("Buffer is too small for gcm");
        exit(1);
    }
     
    gcm(msg,lenMsg,key,&result);


    sprintf(buff,"%03x",numBlocks*32);
    memcpy(&buff[3],result.ciphertext,32*numBlocks);
    sprintf(&buff[32*numBlocks+3],"%08x%08x%08x%08x%08x%08x%08x%08x",result.iv[0],result.iv[1],result.iv[2],result.iv[3],result.iv[4],result.iv[5],result.iv[6],result.iv[7]);
    sprintf(&buff[32*numBlocks+67],"%08x%08x%08x%08x%08x%08x%08x%08x",result.tag[0],result.tag[1],result.tag[2],result.tag[3],result.tag[4],result.tag[5],result.tag[6],result.tag[7]);
    buff[lenToSend-1] = '\0';
    send(sock,buff,lenToSend,0);

    printf("\nSent gcm encrypted: ");
    for(int i=0;i<lenBuff;i++){
        printf("%c",buff[i]); //if you print the string it stops at a 0
    }
    printf("\nSend tag: ");
    for(int i=0;i<8;i++){
        printf("%lu",result.tag[i]);
    }

    free(result.tag);
    free(result.iv);
    free(result.ciphertext);
 
}


void gcmReceiveMessage(int sock,char *buffer,int lenBuff,uint32_t *key){
    memset(buffer,0,lenBuff); //Remove any rubbish from buffer
    if(!recv(sock,buffer,lenBuff,0) || buffer[0]==0){
        printf("\nNo message received from the client");
    }
    else{
        gcmResult result;
        buffer[lenBuff-1] = '\0';
        printf("\nBuffer received %s",buffer); //Won't always print everything as it will stop at a 0
        char temp = buffer[3];
        buffer[3] = '\0';
        int lenMsg = strtoul(buffer,NULL,16);
        buffer[3] = temp;
        result.iv = (uint32_t*) calloc(8,sizeof(uint32_t));
        result.tag  = (uint32_t*) calloc(8,sizeof(uint32_t));
        uchar *message = (uchar*) calloc(lenMsg,sizeof(uchar));
        if(!message || !result.iv || !result.tag){
            if(message) free(message);
            if(result.iv) free(result.iv);
            if(result.tag) free(result.tag);
            perror("\nCalloc error");
            exit(1);
        }
        for(int i=0;i<lenMsg;i++){
            //Copy ciphertext into message
            message[i] = buffer[i+3];
        }
        for(int i=0;i<8;i++){
            //Turn hex strings into uint32_t arrays
            //8 chars for every item
            //8 lots of 8 chars
            //TODO make better strtoul
            temp = buffer[3+lenMsg+(i<<3)+8];
            buffer[3+lenMsg+(i<<3)+8] = '\0';
            result.iv[i] = strtoul(&buffer[3+lenMsg+(i<<3)],NULL,16); //stroul sets the end point, you do not; you must provide it with null terminated string
            buffer[3+lenMsg+(i<<3)+8] = temp;
            temp = buffer[67+lenMsg+(i<<3)+8];
            buffer[67+lenMsg+(i<<3)+8] = '\0';
            result.tag[i] = strtoul(&buffer[67+lenMsg+(i<<3)],NULL,16); 
            buffer[67+lenMsg+(i<<3)+8] = temp;
            //The tag is checked by gcm
        }
        if(gcm(message,lenMsg,key,&result)){
            printf("\nGcm decrypted: ");
            for(int i=0;i<lenMsg;i++){
                printf("%c",result.ciphertext[i]); //if you print the string it stops at a 0
            }
        }

        free(result.tag);
        free(result.iv);
        free(result.ciphertext);
    }
}
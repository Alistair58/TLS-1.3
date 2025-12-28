#ifndef MSG_H
#define MSG_H

#include <stdint.h>
#include "globals.h"

void ecbSendMessage(int sock,uchar *buffer,int lenBuff,uint32_t *key,char *msg,int lenMsg);
void ecbReceiveMessage(int sock,char *buffer, int lenBuff, uint32_t *key);
void gcmSendMessage(int sock,uchar *buff,int lenBuff,uint32_t *key,char *msg,int lenMsg);
void gcmReceiveMessage(int sock,char *buffer, int lenBuff, uint32_t *key);

#endif MSG_H
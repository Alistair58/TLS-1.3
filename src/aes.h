#ifndef AES_H
#define AES_H


typedef unsigned char byte;

void aesEncrypt(uint32_t *key,uint32_t *data, uint32_t* dest);
void aesDecrypt(uint32_t *key,uint32_t *data, uint32_t* dest);

#endif


#ifndef GCM_H
#define GCM_H

#include <stdbool.h>

typedef unsigned char uchar;

typedef struct gcmResult{
    uint32_t *iv;
    uchar *ciphertext;
    uint32_t *tag;
} gcmResult;

bool gcm(uchar *plaintext, int lenPlaintext, uint32_t *key, gcmResult *dest);

#endif


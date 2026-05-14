#ifndef SHA1_H
#define SHA1_H

#include <stdint.h>
#include <windows.h>

#define SHA1_HASH_SIZE 20

typedef struct
{
    uint32_t intermediateHash[SHA1_HASH_SIZE / 4];
    uint32_t lengthLow;
    uint32_t lengthHigh;
    int_least16_t messageBlockIndex;
    uint8_t messageBlock[64];
    BOOL computed;
    BOOL corrupted;
} SHA1Context;

void InitialiseSHA1(SHA1Context *context);
BOOL InputToSHA1(SHA1Context *context, const uint8_t *messageArray, size_t length);
BOOL GetSHA1Result(SHA1Context *context, uint8_t messageDigest[SHA1_HASH_SIZE]);

#endif

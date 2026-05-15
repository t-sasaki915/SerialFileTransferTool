#include <stdint.h>
#include <windows.h>

#include "SHA1.h"
#include "Util.h"

#define CIRCULAR_SHIFT(bits, word) (((word) << (bits)) | ((word) >> (32 - (bits))))

#define K_0 (uint32_t)0x5A827999
#define K_1 (uint32_t)0x6ED9EBA1
#define K_2 (uint32_t)0x8F1BBCDC
#define K_3 (uint32_t)0xCA62C1D6

void DecodeSHA1Hash(const uint8_t sha1Bytes[SHA1_HASH_SIZE], wchar_t sha1Hash[SHA1_HASH_TEXT_SIZE])
{
    for (int i = 0; i < SHA1_HASH_SIZE; i++)
    {
        Format(&sha1Hash[i * 2], 3, L"%02X", sha1Bytes[i]);
    }

    sha1Hash[SHA1_HASH_TEXT_SIZE - 1] = '\0';
}

void SHA1ProcessMessageBlock(SHA1Context *context)
{
    int i;

    uint32_t wordSequence[80];

    for (i = 0; i < 16; i++)
    {
        wordSequence[i] = context->messageBlock[i * 4] << 24;
        wordSequence[i] |= context->messageBlock[i * 4 + 1] << 16;
        wordSequence[i] |= context->messageBlock[i * 4 + 2] << 8;
        wordSequence[i] |= context->messageBlock[i * 4 + 3];
    }

    for (i = 16; i < 80; i++)
    {
        wordSequence[i] =
            CIRCULAR_SHIFT(1, wordSequence[i - 3] ^ wordSequence[i - 8] ^ wordSequence[i - 14] ^ wordSequence[i - 16]);
    }

    uint32_t a = context->intermediateHash[0];
    uint32_t b = context->intermediateHash[1];
    uint32_t c = context->intermediateHash[2];
    uint32_t d = context->intermediateHash[3];
    uint32_t e = context->intermediateHash[4];

    uint32_t x;

    for (i = 0; i < 20; i++)
    {
        x = CIRCULAR_SHIFT(5, a) + ((b & c) | ((~b) & d)) + e + wordSequence[i] + K_0;
        e = d;
        d = c;
        c = CIRCULAR_SHIFT(30, b);
        b = a;
        a = x;
    }

    for (i = 20; i < 40; i++)
    {
        x = CIRCULAR_SHIFT(5, a) + (b ^ c ^ d) + e + wordSequence[i] + K_1;
        e = d;
        d = c;
        c = CIRCULAR_SHIFT(30, b);
        b = a;
        a = x;
    }

    for (i = 40; i < 60; i++)
    {
        x = CIRCULAR_SHIFT(5, a) + ((b & c) | (b & d) | (c & d)) + e + wordSequence[i] + K_2;
        e = d;
        d = c;
        c = CIRCULAR_SHIFT(30, b);
        b = a;
        a = x;
    }

    for (i = 60; i < 80; i++)
    {
        x = CIRCULAR_SHIFT(5, a) + (b ^ c ^ d) + e + wordSequence[i] + K_3;
        e = d;
        d = c;
        c = CIRCULAR_SHIFT(30, b);
        b = a;
        a = x;
    }

    context->intermediateHash[0] += a;
    context->intermediateHash[1] += b;
    context->intermediateHash[2] += c;
    context->intermediateHash[3] += d;
    context->intermediateHash[4] += e;

    context->messageBlockIndex = 0;
}

void SHA1PadMessage(SHA1Context *context)
{
    context->messageBlock[context->messageBlockIndex++] = 0x80;

    if (context->messageBlockIndex > 55)
    {
        while (context->messageBlockIndex < 64)
        {
            context->messageBlock[context->messageBlockIndex++] = 0;
        }

        SHA1ProcessMessageBlock(context);

        while (context->messageBlockIndex < 56)
        {
            context->messageBlock[context->messageBlockIndex++] = 0;
        }
    }
    else
    {
        while (context->messageBlockIndex < 56)
        {
            context->messageBlock[context->messageBlockIndex++] = 0;
        }
    }

    context->messageBlock[56] = context->lengthHigh >> 24;
    context->messageBlock[57] = context->lengthHigh >> 16;
    context->messageBlock[58] = context->lengthHigh >> 8;
    context->messageBlock[59] = context->lengthHigh;

    context->messageBlock[60] = context->lengthLow >> 24;
    context->messageBlock[61] = context->lengthLow >> 16;
    context->messageBlock[62] = context->lengthLow >> 8;
    context->messageBlock[63] = context->lengthLow;

    SHA1ProcessMessageBlock(context);
}

void InitialiseSHA1(SHA1Context *context)
{
    context->intermediateHash[0] = 0x67452301;
    context->intermediateHash[1] = 0xEFCDAB89;
    context->intermediateHash[2] = 0x98BADCFE;
    context->intermediateHash[3] = 0x10325476;
    context->intermediateHash[4] = 0xC3D2E1F0;

    context->lengthLow = 0;
    context->lengthHigh = 0;
    context->messageBlockIndex = 0;

    context->computed = FALSE;
    context->corrupted = FALSE;
}

BOOL InputToSHA1(SHA1Context *context, const uint8_t *messageArray, size_t length)
{
    if (length == 0)
    {
        return TRUE;
    }

    if (context->computed)
    {
        context->corrupted = TRUE;

        return FALSE;
    }

    if (context->corrupted)
    {
        return FALSE;
    }

    while (length-- && !context->corrupted)
    {
        context->messageBlock[context->messageBlockIndex++] = (*messageArray & 0xFF);

        context->lengthLow += 8;
        if (context->lengthLow == 0)
        {
            context->lengthHigh++;
            if (context->lengthHigh == 0)
            {
                context->corrupted = TRUE;
            }
        }

        if (context->messageBlockIndex == 64)
        {
            SHA1ProcessMessageBlock(context);
        }

        messageArray++;
    }

    return TRUE;
}

BOOL GetSHA1Result(SHA1Context *context, uint8_t messageDigest[SHA1_HASH_SIZE])
{
    if (context->corrupted)
    {
        return FALSE;
    }

    if (!context->computed)
    {
        SHA1PadMessage(context);

        for (int i = 0; i < 64; i++)
        {
            context->messageBlock[i] = 0;
        }

        context->lengthLow = 0;
        context->lengthHigh = 0;

        context->computed = TRUE;
    }

    for (int i = 0; i < SHA1_HASH_SIZE; i++)
    {
        messageDigest[i] = context->intermediateHash[i >> 2] >> 8 * (3 - (i & 0x03));
    }

    return TRUE;
}

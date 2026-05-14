#include <stdio.h>

#include "../src/SHA1.h"

typedef struct
{
    char *textInput;
    size_t textInputLength;
    char *expectedOutput;
    int repeatCount;
} SHA1TestCase;

#define TEST_COUNT 4

static const SHA1TestCase TEST_CASES[5] = {
    {"abc", 3, "A9993E364706816ABA3E25717850C26C9CD0D89D", 1},
    {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", 56, "84983E441C3BD26EBAAE4AA1F95129E5E54670F1", 1},
    {"a", 1, "34AA973CD4C4DAA4F61EEB2BDBAD27316534016F", 1000000},
    {"0123456701234567012345670123456701234567012345670123456701234567",
     64,
     "DEA356A2CDDD90C7A7ECEDC5EBB563934F460452",
     10}};

void SHA1Test(int *numberOfCases, int *successCount, int *failureCount)
{
    *numberOfCases = TEST_COUNT;
    *successCount = 0;
    *failureCount = 0;

    for (int i = 0; i < TEST_COUNT; i++)
    {
        SHA1TestCase testCase = TEST_CASES[i];
        printf("Test %d: '%s' (Repeat %d)\n", i, testCase.textInput, testCase.repeatCount);

        SHA1Context context;
        InitialiseSHA1(&context);

        for (int j = 0; j < testCase.repeatCount; j++)
        {
            if (!InputToSHA1(&context, (const unsigned char *)testCase.textInput, testCase.textInputLength))
            {
                printf("Test %d: ERROR, InputToSHA1 returned FALSE at repeat count %d.\n", i, j);
                (*failureCount)++;

                goto NextTest;
            }
        }

        uint8_t messageDigest[SHA1_HASH_SIZE];
        if (!GetSHA1Result(&context, messageDigest))
        {
            printf("Test %d: ERROR, GetSHA1Result returned FALSE.\n", i);
            (*failureCount)++;

            goto NextTest;
        }

        char generatedHash[SHA1_HASH_SIZE];
        for (int j = 0; j < SHA1_HASH_SIZE; j++)
        {
            sprintf(&generatedHash[j * 2], "%02X", messageDigest[j]);
        }

        for (int j = 0; j < SHA1_HASH_SIZE; j++)
        {
            if (generatedHash[j] != testCase.expectedOutput[j])
            {
                printf(
                    "Test %d: ERROR, hash mismatch. Expected: %s, Generated: %s.\n",
                    i,
                    testCase.expectedOutput,
                    generatedHash);

                (*failureCount)++;

                goto NextTest;
            }
        }

        printf("Test %d: SUCCESS.\n", i);
        (*successCount)++;

    NextTest:
        printf("\n");
        continue;
    }
}

#include <stdio.h>
#include <windows.h>

#include "SHA1Test.h"

int main()
{
    int numberOfCases, successCount, failureCount;

    printf("---------- SHA1 Test ----------\n");
    SHA1Test(&numberOfCases, &successCount, &failureCount);
    printf("Success: %d, Failure: %d, Total: %d.\n", successCount, failureCount, numberOfCases);
    printf("-------------------------------\n");

    return failureCount != 0 ? 1 : 0;
}

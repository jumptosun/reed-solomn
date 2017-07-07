
#include <rs_galois.hpp>
#include <assert.h>

uint8_t galMultiply(uint8_t a, uint8_t b)
{
    return mulTable[a][b];
}

uint8_t galDivide(uint8_t a, uint8_t b)
{
    if( a == 0 ) {
        return 0;
    }

    assert(b);

    int logA = expTable[a];
    int logB = expTable[b];

    int logResult = logA - logB;

    if(logResult < 0) {
        logResult += 255;
    }

    return expTable[logResult];
}

uint8_t galExp(uint8_t a, int n)
{
    if(n == 0) {
        return 1;
    }

    if(a == 0) {
        return 0;
    }


    int logA = logTable[a];
    int logResult = logA * n;

    logResult %= 255;

    return expTable[logResult];
}

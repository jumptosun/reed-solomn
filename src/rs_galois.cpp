
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

    int logA = logTable[a];
    int logB = logTable[b];

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

void galMulSlice(uint8_t c, iovec *in, iovec *out)
{
    uint8_t* mt = (uint8_t*)mulTable[c];
    uint8_t input = 0;

    for(int i = 0; i < out->iov_len; i++) {
        input = 0;

        if(i < in->iov_len) {
            input = ((uint8_t*)(in->iov_base))[i];
        }

        ((uint8_t*)(out->iov_base))[i] = mt[input];
    }
}

void galMulSliceXor(uint8_t c, iovec *in, iovec *out)
{
    uint8_t* mt = (uint8_t*)mulTable[c];
    uint8_t input = 0;

    for(int i = 0; i < out->iov_len; i++) {
        input = 0;

        if(i < in->iov_len) {
            input = ((uint8_t*)(in->iov_base))[i];
        }

        ((uint8_t*)(out->iov_base))[i] ^= mt[input];
    }
}

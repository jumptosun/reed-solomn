
#include <reed_solomon.hpp>
#include <stddef.h>
#include <stdio.h>
#include <rs_error.hpp>

ReedSolomon::ReedSolomon()
{
    m_nDataShards = -1;
    m_nParityShards = -1;
    m_nShards = -1;

    m_Matrix = NULL;
    m_Parity = NULL;
}

ReedSolomon::~ReedSolomon()
{

}

int ReedSolomon::initialize(int dataShards, int parityShards)
{
    int ret = ERROR_SUCCESS;

    if(m_nDataShards != -1 || m_nParityShards != -1 || m_nShards != -1) {
        fprintf(stderr, "object already initialize");
        return ERROR_INVALID_PARAM;
    }

    if(dataShards <= 0 || parityShards <= 0 || (dataShards + parityShards) > 255) {
        fprintf(stderr, "invalid parameter, should be between 0~255");
        return ERROR_INVALID_PARAM;
    }

    // get matrix
}

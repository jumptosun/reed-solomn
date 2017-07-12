
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <rs_error.hpp>
#include <rs_core.hpp>
#include <reed_solomon.hpp>
#include <rs_galois.hpp>

using namespace std;

ReedSolomon::ReedSolomon()
{
    m_nDataShards = -1;
    m_nParityShards = -1;
    m_nShards = -1;

    m_Matrix = NULL;
    m_Tree = NULL;
    m_Parity = NULL;
}

ReedSolomon::~ReedSolomon()
{
    rs_freep(m_Matrix);
    rs_freep(m_Tree);
    rs_freep(m_Parity);
}

int ReedSolomon::Initialize(int dataShards, int parityShards)
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

    m_nDataShards = dataShards;
    m_nParityShards = parityShards;
    m_nShards = m_nDataShards + m_nParityShards;

    RsMatrix* vm = RsMatrix::VanderMonde(m_nShards, m_nDataShards);
    RsAutoFree(RsMatrix, vm);

    if(vm == NULL) {
        ret = ERROR_INVALID_PARAM;
        fprintf(stderr, "get the vandermonde matrix failed.");
        return ret;
    }

    RsMatrix* top =  vm->SubMatrix(0, 0, m_nDataShards, m_nDataShards);
    RsAutoFree(RsMatrix, top);

    m_Matrix = vm->Multiply(top->Invert());
    m_Tree = new RsInversionTree(m_nShards);

    m_Parity = m_Matrix->SubMatrix(m_nDataShards, 0, m_nShards, m_Matrix->m_nRows);

    return ret;
}

int ReedSolomon::Encode(std::vector<iovec *> &shards)
{
    int ret = ERROR_SUCCESS;
    int maxLength = 0;

    if(shards.size() != m_nDataShards) {
        ret = ERROR_INVALID_PARAM;
        return ret;
    }

    if(checkShards(shards, maxLength) != ERROR_SUCCESS) {
        ret = ERROR_INVALID_PARAM;
        return ret;
    }

    std::vector<iovec*> output;
    for(int i = 0; i < m_nParityShards; i++) {
        iovec* parityShard = new iovec;
        parityShard->iov_base = new uint8_t[maxLength];
        parityShard->iov_len = maxLength;

        output.push_back(parityShard);
    }

    codeSomeShards(m_Parity, shards, output, m_nParityShards);
    shards.insert(shards.end(),output.begin(),output.end());

    return ret;
}

int ReedSolomon::Reconstruct(std::vector<iovec *> &shards, int maxLength)
{
    int ret = ERROR_SUCCESS;
    int numberPresent  = 0;

    if(shards.size() != m_nShards) {
        ret = ERROR_INVALID_PARAM;
        return ret;
    }

    for(int i = 0; i < shards.size(); i++) {
        if(shards[i] != NULL) {
            numberPresent++;
        }
    }

    if(numberPresent == m_nShards) {
        return ret;
    }

    if(numberPresent < m_nDataShards) {
        ret = ERROR_TOO_FEW_SHARDS;
        return ret;
    }

    vector<iovec*> subShards;
    vector<int> validIndice, invalidIndice;

    for(int matrixRow = 0; matrixRow < m_nShards; matrixRow++ ) {

        if(shards[matrixRow] != NULL) {
            subShards.push_back(shards[matrixRow]);
            validIndice.push_back(matrixRow);
        } else {
            invalidIndice.push_back(matrixRow);
        }
    }

    RsMatrix* dataDecodeMatrix = m_Tree->GetInvertedMatrix(invalidIndice);
    RsAutoFree(RsMatrix, dataDecodeMatrix);
    RsMatrix* subMatrix = new RsMatrix();
    RsAutoFree(RsMatrix, subMatrix);

    if(dataDecodeMatrix == NULL) {
        subMatrix->Initialize(m_nDataShards, m_nDataShards);

        for(int row = 0; row < validIndice.size(); row++) {
            for(int c = 0; c < m_nDataShards; c++) {
                subMatrix->m_Matrix[row][c] = m_Matrix->m_Matrix[validIndice[row]][c];
            }
        }

        dataDecodeMatrix = subMatrix->Invert();
        if(m_Tree->InsertInvertedMatrix(invalidIndice,dataDecodeMatrix,m_nShards)) {
            fprintf(stderr,"insert decode matrix failed.");
        }
    }

    vector<iovec*> output;
    RsMatrix* matrixParity = new RsMatrix();
    matrixParity->Initialize(m_nParityShards, m_nDataShards);
    int outputCount = 0;

    for(int iShard = 0; iShard < m_nDataShards; iShard++) {
        if(shards[iShard] == NULL) {
            shards[iShard] = new iovec;
            shards[iShard]->iov_base = new uint8_t[maxLength];
            shards[iShard]->iov_len = maxLength;

            output.push_back(shards[iShard]);
            memcpy(matrixParity->m_Matrix[outputCount], dataDecodeMatrix->m_Matrix[iShard], m_nDataShards);

            outputCount++;
        }
    }

    RsMatrix* matrixRows = matrixParity->SubMatrix(0, 0, outputCount, m_nDataShards);
    RsAutoFree(RsMatrix, matrixRows);

    codeSomeShards(matrixRows, subShards, output, outputCount);

    return ret;
}

int ReedSolomon::checkShards(std::vector<iovec *> &shards, int &maxLength)
{
    int ret = ERROR_SUCCESS;
    maxLength = 0;

    for(int i = 0; i< shards.size(); i++) {
        if(shards[i] == NULL) {
            ret = ERROR_INVALID_PARAM;
            return ret;
        }

        if(shards[i]->iov_len > maxLength) {
            maxLength = shards[i]->iov_len;
        }
    }

    return ERROR_SUCCESS;
}

int ReedSolomon::codeSomeShards(RsMatrix *MatrixRows, std::vector<iovec *> &input, std::vector<iovec *> &output, int outputCount)
{
    int r, c;
    iovec* in;

    for(int c = 0; c < m_nDataShards; c++) {
        in = input[c];

        for(int iRow = 0; iRow < outputCount; iRow++) {
            if( c == 0 ) {
                galMulSlice(MatrixRows->m_Matrix[iRow][c], in, output[iRow]);
            } else {
                galMulSliceXor(MatrixRows->m_Matrix[iRow][c], in, output[iRow]);
            }
        }
    }
}

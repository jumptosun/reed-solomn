
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <rs_error.hpp>
#include <rs_core.hpp>
#include <reed_solomon.hpp>
#include <rs_galois.hpp>
#include <rs_config.hpp>
#include <rs_matrix.hpp>

using namespace std;

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
    rs_freep(m_Matrix);
    rs_freep(m_Parity);

    std::map<std::vector<int>, RsMatrix*>::iterator it = m_Tree.begin();
    for(; it != m_Tree.end(); ++it) {
        rs_freep(it->second);
    }
}

int ReedSolomon::Initialize(int dataShards, int parityShards)
{
    int ret = ERROR_SUCCESS;

    if(m_nDataShards != -1 || m_nParityShards != -1 || m_nShards != -1) {
        rs_log( "object already initialize");
        return ERROR_INVALID_PARAM;
    }

    if(dataShards <= 0 || parityShards <= 0 || (dataShards + parityShards) > 255) {
        rs_log( "invalid parameter, should be between 0~255");
        return ERROR_INVALID_PARAM;
    }

    // assignment
    m_nDataShards = dataShards;
    m_nParityShards = parityShards;
    m_nShards = m_nDataShards + m_nParityShards;

    // Start with a Vandermonde matrix.  This matrix would work,
    // in theory, but doesn't have the property that the data
    // shards are unchanged after encoding.
    RsMatrix* vm = RsMatrix::VanderMonde(m_nShards, m_nDataShards);
    RsAutoFree(RsMatrix, vm);

    if(vm == NULL) {
        ret = ERROR_INVALID_PARAM;
        rs_log( "get the vandermonde matrix failed.");
        return ret;
    }

    // Multiply by the inverse of the top square of the matrix.
    // This will make the top square be the identity matrix, but
    // preserve the property that any square subset of rows  is
    // invertible.
    RsMatrix* top =  vm->SubMatrix(0, 0, m_nDataShards, m_nDataShards);
    RsAutoFree(RsMatrix, top);

    RsMatrix* invert = top->Invert();
    RsAutoFree(RsMatrix, invert);

    m_Matrix = vm->Multiply(invert);

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

    // create the parity shards
    // the output parity shards' length should be the max length of the input
    std::vector<iovec*> output;
    for(int i = 0; i < m_nParityShards; i++) {
        iovec* parityShard = new iovec;
        parityShard->iov_base = new uint8_t[maxLength];
        bzero(parityShard->iov_base, maxLength);
        parityShard->iov_len = maxLength;

        output.push_back(parityShard);
    }

    // do encode
    codeSomeShards(m_Parity, shards, output, maxLength);

    // append the parity shards after the input
    shards.insert(shards.end(),output.begin(),output.end());

    return ret;
}

int ReedSolomon::Reconstruct(std::vector<iovec *> &shards, bool reconstruct_parity)
{
    int ret = ERROR_SUCCESS;
    int numberPresent  = 0;
    int maxLength = 0;

    // check arguments
    if(shards.size() != m_nShards) {
        ret = ERROR_INVALID_PARAM;
        return ret;
    }

    for(int i = 0; i < shards.size(); i++) {
        if(shards[i] != NULL) {
            numberPresent++;

            if(maxLength < shards[i]->iov_len) {
                maxLength = shards[i]->iov_len;
            }
        }
    }

    // Quick check: are all of the shards present?  If so, there's
    // nothing to do.
    if(numberPresent == m_nShards) {
        return ret;
    }

    // More complete sanity check
    if(numberPresent < m_nDataShards) {
        ret = ERROR_TOO_FEW_SHARDS;
        return ret;
    }

    // Pull out an array holding just the shards that
    // correspond to the rows of the submatrix.  These shards
    // will be the input to the decoding process that re-creates
    // the missing data shards.
    //
    // Also, create an array of indices of the valid rows we do have
    // and the invalid rows we don't have up until we have enough valid rows.
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

    // If the inverted matrix isn't cached in the tree yet we must
    // construct it ourselves and insert it into the tree for the
    // future.  In this way the inversion tree is lazily loaded.
    RsMatrix* dataDecodeMatrix = NULL;
    std::map<std::vector<int>, RsMatrix*>::iterator it;
    if((it = m_Tree.find(invalidIndice)) != m_Tree.end()) {
        dataDecodeMatrix = it->second;
    }

    if(dataDecodeMatrix == NULL) {
        RsMatrix subMatrix;
        subMatrix.Initialize(m_nDataShards, m_nDataShards);

        // Pull out the rows of the matrix that correspond to the
        // shards that we have and build a square matrix.  This
        // matrix could be used to generate the shards that we have
        // from the original data.
        for(int row = 0; row < validIndice.size() && row < subMatrix.m_nRows; row++) {
            memcpy(subMatrix.m_Matrix[row], m_Matrix->m_Matrix[validIndice[row]], m_nDataShards);
        }

        // Invert the matrix, so we can go from the encoded shards
        // back to the original data.  Then pull out the row that
        // generates the shard that we want to decode.  Note that
        // since this matrix maps back to the original data, it can
        // be used to create a data shard, but not a parity shard.
        dataDecodeMatrix = subMatrix.Invert();

        // Cache the inverted matrix in the tree for future use keyed on the
        // indices of the invalid rows.
        m_Tree[invalidIndice] = dataDecodeMatrix;
    }

    // Re-create any data shards that were missing.
    //
    // The input to the coding is all of the shards we actually
    // have, and the output is the missing data shards.  The computation
    // is done using the special decode matrix we just built.

    vector<iovec*> output;
    int outputCount = 0;

    RsMatrix matrixRows;
    matrixRows.Initialize(m_nParityShards, m_nDataShards);//3*7

    for(int iShard = 0; iShard < m_nDataShards; iShard++) {
        if(shards[iShard] == NULL) {
            shards[iShard] = new iovec;
            shards[iShard]->iov_base = new uint8_t[maxLength];
            bzero(shards[iShard]->iov_base, maxLength);
            shards[iShard]->iov_len = maxLength;

            output.push_back(shards[iShard]);
            memcpy(matrixRows.m_Matrix[outputCount], dataDecodeMatrix->m_Matrix[iShard], m_nDataShards);

            outputCount++;
        }
    }

    codeSomeShards(&matrixRows, subShards, output, maxLength);

    if(!reconstruct_parity) {
        return ret;
    }

    // reconstruct parity
    output.clear();
    outputCount = 0;

    for(int iShard = m_nDataShards; iShard < m_nShards; iShard++) {
        if(shards[iShard] == NULL) {
            shards[iShard] = new iovec;
            shards[iShard]->iov_base = new uint8_t[maxLength];
            bzero(shards[iShard]->iov_base, maxLength);
            shards[iShard]->iov_len = maxLength;

            output.push_back(shards[iShard]);
            memcpy(matrixRows.m_Matrix[outputCount], m_Parity->m_Matrix[iShard - m_nDataShards], m_nDataShards);

            outputCount++;
        }
    }

    vector<iovec*> in;
    for(int i = 0; i < m_nDataShards; i++){
        in.push_back(shards[i]);
    }

    codeSomeShards(&matrixRows, in, output, maxLength);

    return ret;
}

bool ReedSolomon::Verify(std::vector<iovec *> &shards){
    if (shards.size() != m_nShards){
        return false;
    }

    int maxLength = 0;

    for(int i = 0; i < m_nDataShards; i++) {
        if(shards[i] != NULL) {

            if(maxLength < shards[i]->iov_len) {
                maxLength = shards[i]->iov_len;
            }
        }
    }

    // outputs is generated tmply, to compare with shards[7:]
    vector<iovec *> outputs;
    for(int i = 0; i < m_nParityShards; i++){
        iovec* data = new iovec;
        data->iov_base = new uint8_t[maxLength];
        bzero(data->iov_base, maxLength);
        data->iov_len = maxLength;
        outputs.push_back(data);
    }
    for(int i = 0; i < m_nDataShards; i++){
        iovec* data = shards[i];
        for(int jRow = 0; jRow < m_nParityShards; jRow++){
            galMulSliceXor(m_Parity->m_Matrix[jRow][i], data, outputs[jRow], maxLength);
        }
    }
    //int n = 0;
    for(int row = 0; row < outputs.size(); row++ ){
        for(int i = 0; i < maxLength; i++){
            int a = ((uint8_t*)(outputs[row]->iov_base))[i];
            int b = ((uint8_t*)(shards[row+m_nDataShards]->iov_base))[i];
            if(a != b){
                printf("verify not pass \n");
                return false;
            }
        }
    }

    return true;


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

int ReedSolomon::codeSomeShards(RsMatrix *MatrixRows, std::vector<iovec *> &input, std::vector<iovec *> &output, int byteCount)
{
    int r, c;
    iovec* in;

    for(int c = 0; c < m_nDataShards; c++) {
        in = input[c];

        for(int iRow = 0; iRow < output.size(); iRow++) {
            if( c == 0 ) {
                galMulSlice(MatrixRows->m_Matrix[iRow][c], in, output[iRow], byteCount);
            } else {
                galMulSliceXor(MatrixRows->m_Matrix[iRow][c], in, output[iRow], byteCount);
            }
        }
    }
}

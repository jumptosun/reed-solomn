#include "rs_xor.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <rs_error.hpp>
#include <rs_core.hpp>
#include <rs_config.hpp>

rs_xor::rs_xor()
{
    m_nDataShards = -1;
    m_nParityShards = -1;
    m_nShards = -1;
}

int rs_xor::Initialize(int dataShards, int parityShards){

    int ret = ERROR_SUCCESS;

    if(m_nDataShards != -1 || m_nParityShards != -1 || m_nShards != -1) {
        rs_log( "object already initialize");
        return ERROR_INVALID_PARAM;
    }

    if(dataShards <= 0 || parityShards != 1 || (dataShards + parityShards) > 255) {
        rs_log( "invalid parameter, should be between 0~255, parityShards should be 1");
        return ERROR_INVALID_PARAM;
    }

    m_nDataShards = dataShards;
    m_nParityShards = parityShards;
    m_nShards = m_nDataShards + m_nParityShards;

    return ret;
}

int rs_xor::Encode(std::vector<iovec *> &shards){
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
    iovec* parityShard = new iovec;
    parityShard->iov_base = new uint8_t[maxLength];
    parityShard->iov_len = maxLength;
    bzero(parityShard->iov_base, parityShard->iov_len);

    // do encode
//    uint8_t sum;//xor sum of each row's value of index i
//    iovec * first_row = shards[0];
//    for(int i = 0; i < maxLength; i++){
//        sum = i < first_row->iov_len ? ((uint8_t*)first_row->iov_base)[i] : 0;
//        for(int row = 1; row < m_nDataShards; row++){
//            iovec * r = shards[row];
//            uint8_t value = i < r->iov_len ? ((uint8_t*)r->iov_base)[i] : 0;// fill 0 if row > iov_len
//            sum ^= value;
//        }
//        ((uint8_t*)parityShard->iov_base)[i] = sum;
//    }



    iovec* eachShard = NULL;
    uint32_t *data, *fec;
    for(int i = 0; i < m_nDataShards; i++) {
        eachShard = shards[i];

        for(int j = 0; j < eachShard->iov_len; j += 4) {
            data = (uint32_t*)((char*)eachShard->iov_base + j);
            fec = (uint32_t*)((char*)parityShard->iov_base + j);

            *fec ^= *data;
        }

        int tail = eachShard->iov_len % 4;
        for(int k = eachShard->iov_len - tail; k <  eachShard->iov_len; k++) {
            ((char*)parityShard->iov_base)[k] ^= ((char*)eachShard->iov_base)[k];
        }
    }


    // append the parity shard after the input
    shards.push_back(parityShard);

    return ERROR_SUCCESS;
}

int rs_xor::Reconstruct(std::vector<iovec *> &shards, bool reconstruct_parity)
{
    int ret = ERROR_SUCCESS;
    // check arguments
    int maxLength = 0;//the max int rows' length
    int nlost_row = -1;
    if(shards.size() != m_nShards) {
        ret = ERROR_INVALID_PARAM;
        return ret;
    }

    //find the maxLength and the lost_row
    for(int i = 0; i < m_nShards; i++){
        iovec* r = shards[i];
        if( r != NULL){
            if(maxLength < r->iov_len){
                maxLength = r->iov_len;
            }
        }else{
            nlost_row = i;
        }
    }

    iovec* XorShard = new iovec;
    XorShard->iov_base = new uint8_t[maxLength];
    XorShard->iov_len = maxLength;

    // do reconstruct

    uint8_t sum;//xor sum of each row's value of index i
    iovec * first_row;
    int nfirst_row;
    //find first_row
    if(nlost_row == 0){
        first_row = shards[1];
        nfirst_row = 1;
    }else{
        first_row = shards[0];
        nfirst_row = 0;
    }
    for(int i = 0; i < maxLength; i++){

        sum = i < first_row->iov_len ? ((uint8_t*)first_row->iov_base)[i] : 0;

        if(nfirst_row == 0){
            for(int row_befor = 1; row_befor < nlost_row; row_befor++){//row_befor : befor the nlost_row
                iovec * r = shards[row_befor];
                uint8_t value = i < r->iov_len ? ((uint8_t*)r->iov_base)[i] : 0;// fill 0 if row > iov_len
                sum ^= value;
            }
            for(int row_after = nlost_row + 1; row_after < m_nShards; row_after++){
                iovec * r = shards[row_after];
                uint8_t value = i < r->iov_len ? ((uint8_t*)r->iov_base)[i] : 0;// fill 0 if row > iov_len
                sum ^= value;
            }
        }else{
            for(int row = 2; row < m_nShards; row++){
                iovec * r = shards[row];
                uint8_t value = i < r->iov_len ? ((uint8_t*)r->iov_base)[i] : 0;// fill 0 if row > iov_len
                sum ^= value;
            }
        }

        ((uint8_t*)XorShard->iov_base)[i] = sum;

    }

    //replace NULL to XorShard in shards
    shards[nlost_row] = XorShard;

    return ERROR_SUCCESS;
}

int rs_xor::checkShards(std::vector<iovec *> &shards, int &maxLength)
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


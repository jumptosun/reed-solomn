#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>

#include <reed_solomon.hpp>

using namespace std;

#define RS_DATA_SHARDS_NUM		7
#define RS_PARITY_SHARDS_NUM	3
#define RS_SHARDS_NUM			10

int main(int argc, char *argv[])
{
    vector<iovec*> origin;

    printf("before the reconstruct:\n");
    for(int i = 0; i < RS_DATA_SHARDS_NUM; i++) {
        iovec *data = new iovec;
        data->iov_base = new char[64];
        data->iov_len = 64;

        bzero(data->iov_base,64);
        sprintf((char*)data->iov_base,"%d %d %d", i, i+1 , i+2);
        printf("%s\n",(char*)data->iov_base);

        origin.push_back(data);
    }
    cout<<endl;

    ReedSolomon* rs = new ReedSolomon();
    rs->Initialize(RS_DATA_SHARDS_NUM, RS_PARITY_SHARDS_NUM);

    // encode
    rs->Encode(origin);

    // drop the first 3 data
    printf("drop first 3 data.\n\n");
    for(int i = 0; i < 3; i++) {
        iovec* v = origin[i];
        origin[i] = NULL;

        uint8_t* data = (uint8_t*)v->iov_base;
        delete[] data;
        delete v;
    }

    // reconstruct
    rs->Reconstruct(origin);

    printf("after the reconstruct:\n");
    for(int i = 0; i < 10; i++) {
        printf("%s\n",(char*)origin[i]->iov_base);
    }

    return 0;
}

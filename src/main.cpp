#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>

#include <reed_solomon.hpp>
#include <rs_core.hpp>

using namespace std;

int main(int argc, char *argv[])
{
    vector<iovec*> origin;

    printf("before the reconstruct:\n");
    for(int i = 0; i < 10; i++) {
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
    rs->Initialize(10,4);

    // encode
    rs->Encode(origin);

    printf("drop first 3 data.\n\n");
    for(int i = 0; i < 3; i++) {
        iovec* v = origin[i];
        origin[i] = NULL;

        uint8_t* data = (uint8_t*)v->iov_base;
        rs_freep(data);
        rs_freep(v);
    }

    // reconstruct
    rs->Reconstruct(origin,64);

    printf("after the reconstruct:\n");
    for(int i = 0; i < 10; i++) {
        printf("%s\n",(char*)origin[i]->iov_base);
    }

    return 0;
}

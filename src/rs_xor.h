#ifndef RS_XOR_H
#define RS_XOR_H

#include <ifec.h>

class rs_xor : public IFEC
{
public:
    rs_xor();
    int Initialize(int dataShards, int parityShards);

    int Encode(std::vector<iovec*>& shards);

    int Reconstruct(std::vector<iovec*>& shards, bool reconstruct_parity = false);

protected:
    /**
     * @brief checkShards
     * 	using in the Encode to check the data invalid, and return the max length of the shards.
     */
    int checkShards(std::vector<iovec*>& shards, int& maxLength);


};

#endif // RS_XOR_H

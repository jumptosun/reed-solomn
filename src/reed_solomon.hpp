#ifndef REED_SOLOMON_CPP
#define REED_SOLOMON_CPP

#include <vector>
#include <stdint.h>

class ReedSolomon
{
private:
    int m_nDataShards;
    int m_nParityShards;
    int m_nShards;
    uint8_t** m_Matrix;
    uint8_t** m_Parity;

public:
    ReedSolomon();
    virtual ~ReedSolomon();
public:
    virtual int initialize(int dataShards, int parityShards);

    virtual int Encode(std::vector<uint8_t*>& shards) {}
    virtual bool Verify(std::vector<uint8_t*>& shards) {}
    virtual int Reconstruct(std::vector<uint8_t*>& shards) {}
};

#endif // REED_SOLOMON_CPP

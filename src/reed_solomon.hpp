#ifndef REED_SOLOMON_CPP
#define REED_SOLOMON_CPP

#include <vector>
#include <stdint.h>
#include <sys/socket.h>

#include <rs_matrix.hpp>
#include <rs_inversion_tree.hpp>

class ReedSolomon
{
private:
    int m_nDataShards;
    int m_nParityShards;
    int m_nShards;

    RsMatrix* m_Matrix;
    RsInversionTree* m_Tree;

    RsMatrix* m_Parity;

public:
    ReedSolomon();
    virtual ~ReedSolomon();
public:
    int Initialize(int dataShards, int parityShards);

    int Encode(std::vector<iovec*>& shards);
//    bool Verify(std::vector<iovec*>& shards);
    int Reconstruct(std::vector<iovec*>& shards, int length);
protected:
    int checkShards(std::vector<iovec*>& shards, int& maxLength);
    int codeSomeShards(RsMatrix* MatrixRows, std::vector<iovec*>& input, std::vector<iovec*>& output, int outputCount);
};

#endif // REED_SOLOMON_CPP

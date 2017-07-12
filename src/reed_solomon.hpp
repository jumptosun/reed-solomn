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
    /**
     * @brief Encode
     * the shards can be defferent length,
     * the actual encode length should be the max length of the shards.
     */
    int Encode(std::vector<iovec*>& shards);

    /**
     * @brief Reconstruct
     * if the shards is lost, the shard should be NULL pointer.
     *
     * @param maxLength The encode length of the shards,
     * the lost shard will be reconstruct to the length.
     */
    int Reconstruct(std::vector<iovec*>& shards, int maxLength);

    ///< bool Verify(std::vector<iovec*>& shards);	implement interface if need
protected:
    int checkShards(std::vector<iovec*>& shards, int& maxLength);
    int codeSomeShards(RsMatrix* MatrixRows, std::vector<iovec*>& input, std::vector<iovec*>& output, int outputCount);
};

#endif // REED_SOLOMON_CPP

#ifndef REED_SOLOMON_CPP
#define REED_SOLOMON_CPP

#include <map>
#include <vector>
#include <stdint.h>
#include <sys/socket.h>

class RsMatrix;
class RsInversionTree;

/**
 * @brief The ReedSolomon class
 * 	contains a matrix for a specific
 * 	distribution of datashards and parity shards.
 */
class ReedSolomon
{
private:
    int m_nDataShards;		///< number of the data shards, should not be modified
    int m_nParityShards;	///< Number of parity shards, should not be modified.
    int m_nShards;			///< Total number of shards. Calculated, and should not be modified.

    RsMatrix* m_Matrix;
//    RsInversionTree* m_Tree;
    std::map<std::vector<int>, RsMatrix*> m_Tree;

    RsMatrix* m_Parity;

public:
    ReedSolomon();
    virtual ~ReedSolomon();
public:
    /**
     * @brief Initialize
     * 	create the two-dimensional array m_Matrix according to the input.
     *
     * @param dataShards Encode input data shards number
     * @param parityShards Encode output parity shards number
     */
    int Initialize(int dataShards, int parityShards);
    /**
     * @brief Encode
     * the shards can be defferent length,
     * the actual encode parity length should be the max length of the shards.
     *
     * @param shards The shards size should be equal to m_nDataShards.
     */
    int Encode(std::vector<iovec*>& shards);

    /**
     * @brief Reconstruct
     * Reconstruct will recreate the missing shards, if possible.
     *
     * @param maxLength The encode length of the shards,
     * the lost shard will be reconstruct to the maxlength.
     */
    int Reconstruct(std::vector<iovec*>& shards, int maxLength);

protected:
    /**
     * @brief checkShards
     * 	using in the Encode to check the data invalid, and return the max length of the shards.
     */
    int checkShards(std::vector<iovec*>& shards, int& maxLength);
    /**
     * @brief codeSomeShards
     *  Multiplies a subset of rows from a coding matrix by a full set of
     *  input shards to produce some output shards.
     * @param MatrixRows Is The rows from the matrix to use.
     * @param input  An array of byte arrays, each of which is one input shard.
     * @param output  An array of byte arrays, each of which is a parity shard.
     * @param outputCount Which is the number of outputs to compute, namely the maxLength in Reconstruct
     */
    int codeSomeShards(RsMatrix* MatrixRows, std::vector<iovec*>& input, std::vector<iovec*>& output, int outputCount);
};

#endif // REED_SOLOMON_CPP

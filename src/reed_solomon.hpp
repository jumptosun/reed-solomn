#ifndef REED_SOLOMON_CPP
#define REED_SOLOMON_CPP

//#include <map>
//#include <vector>
//#include <stdint.h>
//#include <sys/socket.h>

#include <ifec.h>

class RsMatrix;
class RsInversionTree;

/**
 * @brief The ReedSolomon class
 * 	contains a matrix for a specific
 * 	distribution of datashards and parity shards.
 */
class ReedSolomon : public IFEC
{
private:
    RsMatrix* m_Matrix;
    std::map<std::vector<int>, RsMatrix*> m_Tree;

    RsMatrix* m_Parity;

public:
    ReedSolomon();
    ~ReedSolomon();
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
    int Reconstruct(std::vector<iovec*>& shards, bool reconstruct_parity = false);
    /**
     * @brief Verify
     * Verify returns true if the parity shards contain correct data.
     * The data is the same format as Encode. No data is modified, so
     * you are allowed to read from data while this is running.
     */
    bool Verify(std::vector<iovec*>& shards);

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
     * @param byteCount
     */
    int codeSomeShards(RsMatrix* MatrixRows, std::vector<iovec*>& input, std::vector<iovec*>& output, int byteCount);
};

#endif // REED_SOLOMON_CPP

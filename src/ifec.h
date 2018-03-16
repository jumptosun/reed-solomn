#ifndef IFEC_H
#define IFEC_H

#include <map>
#include <vector>
#include <stdint.h>
#include <sys/socket.h>

class IFEC
{
public:
    int m_nDataShards;		///< number of the data shards, should not be modified
    int m_nParityShards;	///< Number of parity shards, should not be modified.
    int m_nShards;			///< Total number of shards. Calculated, and should not be modified.
public:
    IFEC();
    virtual ~IFEC();
    /**
     * @brief Initialize
     * 	initial
     *
     * @param dataShards Encode input data shards number
     * @param parityShards Encode output parity shards number
     */
    virtual int Initialize(int dataShards, int parityShards) = 0;
    /**
     * @brief Encode
     * the shards can be defferent length,
     * the actual encode parity length should be the max length of the shards.
     *
     * @param shards The shards size should be equal to m_nDataShards.
     */
    virtual int Encode(std::vector<iovec*>& shards) = 0;
    /**
     * @brief Reconstruct
     * Reconstruct will recreate the missing shards, if possible.
     *
     * @param maxLength The encode length of the shards,
     * the lost shard will be reconstruct to the maxlength.
     */
    virtual int Reconstruct(std::vector<iovec*>& shards, bool reconstruct_parity = false) = 0;

};

#endif // IFEC_H

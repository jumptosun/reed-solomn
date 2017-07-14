#ifndef RS_INVERSION_TREE_HPP
#define RS_INVERSION_TREE_HPP

#include <vector>
#include <pthread.h>

#include <rs_matrix.hpp>

class RsInversionNode
{
public:
    int m_nShards;
    RsInversionNode** m_Children;
    RsMatrix* m_Matrix;
public:
    RsInversionNode(int shards);
    virtual ~RsInversionNode();
public:
    RsMatrix* GetInvertedMatrix(std::vector<int>& invalidIndices, int parent);
    int InsertInvertedMatrix(std::vector<int>& invalidIndices, RsMatrix* matrix, int shards, int parent);
};


class RsInversionTree
{

#ifdef RS_CONFIG_OPTION_THREAD_SAFETY
    pthread_mutex_t m_Mutex;
#endif

public:
    int m_nShards;
    RsInversionNode* root;

public:
    RsInversionTree(int shards);
    virtual ~RsInversionTree();
public:
    RsMatrix* GetInvertedMatrix(std::vector<int>& invalidIndices);
    int InsertInvertedMatrix(std::vector<int>& invalidIndices, RsMatrix* matrix, int shards);
};

#endif // RS_INVERSION_TREE_HPP

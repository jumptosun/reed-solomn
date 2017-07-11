
#include <string.h>
#include <stddef.h>
#include <stdio.h>

#include <rs_error.hpp>
#include <rs_core.hpp>
#include <rs_inversion_tree.hpp>

RsInversionNode::RsInversionNode(int shards)
{
    m_nShards = shards;
    m_Children = new RsInversionNode*[shards];
    bzero(m_Children, shards * sizeof(void*));

    m_Matrix = NULL;
}

RsInversionNode::~RsInversionNode()
{
    for(int i = 0; i < m_nShards; i++) {
        rs_freep(m_Children[i]);
    }

    rs_freep(m_Matrix);
}

RsMatrix *RsInversionNode::GetInvertedMatrix(std::vector<int> &invalidIndices, int parent)
{
    int firstIndex = invalidIndices[0];
    RsInversionNode* node = m_Children[firstIndex - parent];

    if(node == NULL) {
        return NULL;
    }

    if(invalidIndices.size() > 1) {
        std::vector<int> indices(++invalidIndices.begin(), invalidIndices.end());

        return node->GetInvertedMatrix(indices, firstIndex + 1);
    }

    return m_Matrix;
}

int RsInversionNode::InsertInvertedMatrix(std::vector<int> &invalidIndices, RsMatrix *matrix, int shards, int parent)
{
    int ret = ERROR_SUCCESS;
    int firstIndex = invalidIndices[0];
    RsInversionNode* node = m_Children[firstIndex - parent];

    if(node == NULL) {
        node = new RsInversionNode(shards - firstIndex);

        m_Children[firstIndex - parent] = node;
    }

    if(invalidIndices.size() > 1) {
        std::vector<int> indices(++invalidIndices.begin(), invalidIndices.end());

        return node->InsertInvertedMatrix(indices, matrix, shards, firstIndex + 1);
    }

    m_Matrix = matrix;
    return ret;
}

RsInversionTree::RsInversionTree(int shards)
{
    m_nShards = shards;
    root = new RsInversionNode(shards);
}

RsInversionTree::~RsInversionTree()
{
    rs_freep(root);
}

RsMatrix *RsInversionTree::GetInvertedMatrix(std::vector<int> &invalidIndices)
{
    if(invalidIndices.empty()) {
        return NULL;
    }

    return root->GetInvertedMatrix(invalidIndices,0);
}

int RsInversionTree::InsertInvertedMatrix(std::vector<int> &invalidIndices, RsMatrix *matrix, int shards)
{
    int ret = ERROR_SUCCESS;

    if(invalidIndices.empty()) {
        return ret;
    }

    if(!matrix->IsSquare()) {
        ret = ERROR_MATRIX_NOT_SQUARE;
        fprintf(stderr, "the inversion matrix is not square.");
        return ret;
    }

    return root->InsertInvertedMatrix(invalidIndices,matrix,shards,0);
}

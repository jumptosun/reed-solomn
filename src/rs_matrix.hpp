#ifndef RS_MATRIX_HPP
#define RS_MATRIX_HPP

#include <string>
#include <stdint.h>

class RsMatrix
{
public:
    int m_nRows;
    int m_nCols;
    uint8_t** m_Matrix;
public:
    RsMatrix();
    virtual ~RsMatrix();

public:
    static RsMatrix* IdentityMatrix(int size);
    static RsMatrix* VanderMonde(int rows, int cols);

public:
    int Initialize(int rows, int cols);

    bool Check();
    std::string String();
    RsMatrix* Multiply(RsMatrix *right);
    RsMatrix* Augment(RsMatrix *right);
    bool SameSize(RsMatrix* right);
    RsMatrix* SubMatrix(int rmin, int cmin, int rmax, int cmax);
    int SwapRows(int r1, int r2);
    bool IsSquare();
    RsMatrix* Invert();

protected:
    inline int GaussianElimination();
};

#endif // RS_MATRIX_HPP

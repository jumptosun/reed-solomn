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
    virtual int Initialize(int rows, int cols);

    virtual bool Check();
    virtual std::string String();
    virtual RsMatrix* Multiply(RsMatrix *right);
    virtual RsMatrix* Augment(RsMatrix *right);
    virtual bool SameSize(RsMatrix* right);
    virtual RsMatrix* SubMatrix(int rmin, int cmin, int rmax, int cmax);
    virtual int SwapRows(int r1, int r2);
    virtual bool IsSquare();
    virtual RsMatrix* Invert();

protected:
    inline int GaussianElimination();
};

#endif // RS_MATRIX_HPP

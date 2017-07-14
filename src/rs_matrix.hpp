#ifndef RS_MATRIX_HPP
#define RS_MATRIX_HPP

#include <string>
#include <stdint.h>

class RsMatrix
{
public:
    int m_nRows;	///< number of rows
    int m_nCols;	///< number of column
    uint8_t** m_Matrix;	///< the matrix data
public:
    RsMatrix();
    virtual ~RsMatrix();

public:
    /**
     * @brief IdentityMatrix
     *  IdentityMatrix returns an identity matrix of the given size.
     */
    static RsMatrix* IdentityMatrix(int size);
    /**
     * @brief VanderMonde
     * Create a Vandermonde matrix, which is guaranteed to have the
     * property that any subset of rows that forms a square matrix
     * is invertible.
     */
    static RsMatrix* VanderMonde(int rows, int cols);

public:
    /**
     * @brief Initialize
     *  create the matrix with specified row and column
     */
    int Initialize(int rows, int cols);

    bool Check();
    /**
     * @brief String
     *  String returns a human-readable string of the matrix contents.
     */
    std::string String();
    /**
     * @brief Multiply
     *  Multiply multiplies this matrix (the one on the left) by another
     *  matrix (the one on the right) and returns a new matrix with the result.
     */
    RsMatrix* Multiply(RsMatrix *right);
    /**
     * @brief Augment
     *  Augment returns the concatenation of this matrix and the matrix on the right.
     */
    RsMatrix* Augment(RsMatrix *right);

    bool SameSize(RsMatrix* right);
    /**
     * @brief SubMatrix
     *  Returns a part of this matrix. Data is copied.
     */
    RsMatrix* SubMatrix(int rmin, int cmin, int rmax, int cmax);
    /**
     * @brief SwapRows
     *  SwapRows Exchanges two rows in the matrix.
     */
    int SwapRows(int r1, int r2);
    /**
     * @brief IsSquare
     * IsSquare will return true if the matrix is square
     */
    bool IsSquare();
    /**
     * @brief Invert
     * Invert returns the inverse of this matrix.
     * Returns ErrSingular when the matrix is singular and doesn't have an inverse.
     * The matrix must be square, otherwise ErrNotSquare is returned.
     */
    RsMatrix* Invert();

protected:
    inline int gaussianElimination();
};

#endif // RS_MATRIX_HPP

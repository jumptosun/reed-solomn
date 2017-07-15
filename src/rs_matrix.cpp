
#include <rs_matrix.hpp>
#include <rs_error.hpp>
#include <rs_galois.hpp>
#include <rs_core.hpp>
#include <rs_config.hpp>

#include <string.h>
#include <stdio.h>

using namespace std;

RsMatrix::RsMatrix()
{
    m_nRows = -1;
    m_nCols = -1;
    m_Matrix = NULL;
}

RsMatrix::~RsMatrix()
{
    if(m_Matrix != NULL) {
        uint8_t* data = m_Matrix[0];
        rs_freepa(data);
    }
}

RsMatrix *RsMatrix::IdentityMatrix(int size)
{
    if(size <=0 ) {
        return NULL;
    }

    RsMatrix* matrix = new RsMatrix();
    matrix->Initialize(size,size);

    for(int i = 0; i < size; i++) {
        matrix->m_Matrix[i][i] = 1;
    }

    return matrix;
}

RsMatrix *RsMatrix::VanderMonde(int rows, int cols)
{
    if(rows <=0 || cols <= 0) {
        return NULL;
    }

    RsMatrix* result = new RsMatrix();
    result->Initialize(rows, cols);

    for(int r = 0; r < rows; r++) {
        for(int c = 0; c < cols; c++) {
            result->m_Matrix[r][c] = galExp(r,c);
        }
    }

    return result;
}

int RsMatrix::Initialize(int rows, int cols)
{
    if(rows <= 0 || cols <= 0) {
        return ERROR_INVALID_PARAM;
    }

    m_nRows = rows;
    m_nCols = cols;

    uint8_t* matrix = new uint8_t[rows * cols];
    bzero(matrix,rows * cols);
    m_Matrix = new uint8_t*[rows];

    for(int i = 0; i < rows; i++) {
        m_Matrix[i] = matrix + (i * cols);
    }

    return ERROR_SUCCESS;
}

bool RsMatrix::Check()
{
    if(m_nRows <= 0 || m_nRows <= 0 || m_Matrix == NULL) {
        return false;
    }

    return true;
}

string RsMatrix::String()
{
    string s;
    char buf[8];
    bzero(buf,8);

    for(int r = 0; r < m_nRows; r++) {
        for(int c = 0; c < m_nCols; c++) {
            sprintf(buf, "%3d ", m_Matrix[r][c]);
            s += buf;
        }
        s += "\n";
    }

    return s;
}

RsMatrix *RsMatrix::Multiply(RsMatrix *right)
{
    if(this->m_nCols != right->m_nRows) {
        rs_log("first matrix's colomn should equal second matrix's row.");
        return NULL;
    }

    RsMatrix* matrix = new RsMatrix();
    uint8_t value;

    matrix->Initialize(this->m_nRows,right->m_nCols);

    for(int r = 0; r < this->m_nRows; r++) {
        for(int c = 0; c < right->m_nCols; c++) {

            value = 0;
            for(int i = 0; i < this->m_nCols; i++) {
                value ^= galMultiply(this->m_Matrix[r][i] , right->m_Matrix[i][c]);
            }

            matrix->m_Matrix[r][c] = value;
        }
    }

    return matrix;
}

RsMatrix *RsMatrix::Augment(RsMatrix *right)
{
    if(this->m_nRows != right->m_nRows) {
        return NULL;
    }

    RsMatrix* result = new RsMatrix();

    result->Initialize(this->m_nRows, this->m_nCols + right->m_nCols);

    for(int r = 0; r < this->m_nRows; r++) {
        for(int c = 0; c <this->m_nCols; c++) {
            result->m_Matrix[r][c] = this->m_Matrix[r][c];
        }

        int cols = this->m_nCols;
        for(int c = 0; c < right->m_nCols; c++) {
            result->m_Matrix[r][c + cols] = right->m_Matrix[r][c];
        }
    }

    return result;
}

bool RsMatrix::SameSize(RsMatrix *right)
{
    return (this->m_nCols == right->m_nCols) && (this->m_nRows == right->m_nRows);
}

RsMatrix *RsMatrix::SubMatrix(int rmin, int cmin, int rmax, int cmax)
{
    if((rmax - rmin) <= 0 || (cmax -cmin) <= 0 ||
       (rmax - rmin) > m_nRows || (cmax - cmax) > m_nCols ) {
        rs_log( "invalid parameter.");
        return NULL;
    }

    RsMatrix* result = new RsMatrix();
    result->Initialize(rmax - rmin, cmax - cmin);

    for(int r = rmin; r < rmax; r++) {
        for(int c = cmin; c < cmax; c++) {
            result->m_Matrix[r - rmin][c - cmin] = this->m_Matrix[r][c];
        }
    }

    return result;
}

int RsMatrix::SwapRows(int r1, int r2)
{
    if(r1 < 0 || r2 < 0 || r1 >= m_nRows || r2 >= m_nRows) {
        return ERROR_INVALID_PARAM;
    }

    if(r1 == r2) return ERROR_SUCCESS;

    uint8_t row[m_nCols];
    memcpy(row, m_Matrix[r1], m_nCols);
    memcpy(m_Matrix[r1], m_Matrix[r2], m_nCols);
    memcpy(m_Matrix[r2], row, m_nCols);

    return ERROR_SUCCESS;
}

bool RsMatrix::IsSquare()
{
    return m_nRows == m_nCols;
}

RsMatrix *RsMatrix::Invert()
{
    if(!IsSquare()) {
        return NULL;
    }

    // refine
    RsMatrix *out, *id;
    RsAutoFree(RsMatrix, out);
    RsAutoFree(RsMatrix, id);

    id = IdentityMatrix(m_nRows);
    out = Augment(id);
    out->gaussianElimination();

    return out->SubMatrix(0, m_nRows, m_nRows, m_nRows * 2);
}

int RsMatrix::gaussianElimination()
{
    int ret = ERROR_SUCCESS;
    int r, c, d, rowBelow, rowAbove, scale;

    // Create a Vandermonde matrix, which is guaranteed to have the
    // property that any subset of rows that forms a square matrix
    // is invertible.
    for(r = 0; r < m_nRows; r++) {
        if(m_Matrix[r][r] == 0) {
            for(rowBelow = r + 1; rowBelow < m_nRows; rowBelow++ ) {
                if(m_Matrix[rowBelow][r] != 0) {
                    SwapRows(r, rowBelow);
                    break;
                }
            }
        }

        if(m_Matrix[r][r] == 0) {
            rs_log("the matrix is singular.\n");
            return ERROR_SINGULAR_MATRIX;
        }

        // scale to 1
        if(m_Matrix[r][r] != 1) {
            scale = galDivide(1, m_Matrix[r][r]);
            for(c = 0; c < m_nCols; c++) {
                m_Matrix[r][c] = galMultiply(m_Matrix[r][c], scale);
            }
        }

        // Make everything below the 1 be a 0 by subtracting
        // a multiple of it.  (Subtraction and addition are
        // both exclusive or in the Galois field.)
        for(rowBelow = r + 1; rowBelow < m_nRows; rowBelow++ ) {
            if(m_Matrix[rowBelow][r] != 0) {
                scale = m_Matrix[rowBelow][r];
                for(c = 0; c < m_nCols; c++) {
                    m_Matrix[rowBelow][c] ^= galMultiply(scale,m_Matrix[r][c]);
                }
            }
        }
    }

    // Now clear the part above the main diagonal.
    for(d = 0; d < m_nRows; d++) {
        for(rowAbove = 0; rowAbove < d; rowAbove++) {
            if(m_Matrix[rowAbove][d] != 0) {
                scale = m_Matrix[rowAbove][d];

                for(c = 0; c < m_nCols; c++) {
                    m_Matrix[rowAbove][c] ^= galMultiply(scale,m_Matrix[d][c]);
                }
            }
        }
    }

    return ret;
}


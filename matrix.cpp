#include <algorithm>
#include <cassert>
#include <cmath>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <iterator>
#include <vector>

#include "augmented_matrix.h"

/* Constructors */
template <typename T, int ROWS, int COLS>
matrix<T, ROWS, COLS>::matrix() { }

template <typename T, int ROWS, int COLS>
matrix<T, ROWS, COLS>::matrix(T x) { e.fill(x); }

template <typename T, int ROWS, int COLS>
matrix<T, ROWS, COLS>::matrix(const std::array<T, ROWS*COLS> arr) : e(arr) {}

/* Basic access and helper functions */
template <typename T, int ROWS, int COLS>
int matrix<T, ROWS, COLS>::get_row_bound() const { return rows; }

template <typename T, int ROWS, int COLS>
int matrix<T, ROWS, COLS>::get_col_bound() const { return cols; }

template <typename T, int ROWS, int COLS>
int matrix<T, ROWS, COLS>::get_rows() const { return rows; }

template <typename T, int ROWS, int COLS>
int matrix<T, ROWS, COLS>::get_cols() const { return cols; }

template <typename T, int ROWS, int COLS>
void matrix<T, ROWS, COLS>::set_row(int i, matrix<T, 1, COLS> r)
{
    for (int j = 0; j < COLS; j++)
    {
        (*this)(i, j) = r(0, j);
    }
}

template <typename T, int ROWS, int COLS>
matrix<T, COLS, ROWS> matrix<T, ROWS, COLS>::transpose() const
{
	matrix<T, COLS, ROWS> o {};
	
	for (int j = 0; j < COLS; j++) 
	{
		for (int i = 0; i < ROWS; i++)
		{
			o(j, i) = (*this)(i, j);
		}
	}
	return o;
}

template <typename T, int ROWS, int COLS>
matrix<T, ROWS, COLS> matrix<T, ROWS, COLS>::inverse() const
{
    static_assert(ROWS == COLS, "inverse(): Inverse called on non-square matrix");

    matrix<T, ROWS, COLS> o {};
    augmented<T, ROWS, COLS*2> aug {};
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            aug(i, j) = (*this)(i, j);
        }
    }
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = COLS; j < COLS*2; j++)
        {
            aug(i, j) = (i+COLS == j) ? 1.0 : 0.0;
        }
    }
    aug.set_bounds(0, ROWS, 0, COLS);
    matrix<T, ROWS, COLS*2> augreduced = aug.rref();
    msubmatrix(augreduced, o, 0, 3);
    return o;
}

template<typename T, int ROWS, int COLS>
void matrix<T, ROWS, COLS>::print(std::ostream &out) const 
{
    out << "{";
    for (int i = 0; i < rows; i++) {
        out << "{";
        for (int j = 0; j < cols; j++) {
            out << (*this)(i, j);
            if (j < cols-1) out << ", ";
        }
        out << "}";
        if (i < rows-1) out << ", ";
    }
    out << "}" << std::endl;
}

template<typename T, int ROWS, int COLS>
std::string matrix<T, ROWS, COLS>::template_string() const 
{ 
    return "matrix<" + std::string(typeid(T).name()) + ", " 
    + std::to_string(rows) + ", " + std::to_string(cols) + ">"; 
}

/* Operator overloads */
template <typename T, int ROWS, int COLS>
T matrix<T, ROWS, COLS>::operator()(int i, int j) const { return e.at(i*cols + j); }

template <typename T, int ROWS, int COLS>
T& matrix<T, ROWS, COLS>::operator()(int i, int j) { return e.at(i*cols + j); }

template <typename T, int ROWS, int COLS>
matrix<T, 1, COLS> matrix<T, ROWS, COLS>::operator()(int i) const 
{
    if (i >= rows || i < 0) 
    {
        std::string errormsg = "Row index " + std::to_string(i) + " out of range for " + template_string() + ".";
        throw std::out_of_range(errormsg);
    }
    std::array<T, COLS> vecarray {};
    int start = i*cols;
    std::copy(std::begin(e) + start, std::begin(e) + start + cols, std::begin(vecarray));
    return matrix<T, 1, COLS>(vecarray);
}

template<typename T, int ROWS, int COLS>
std::ostream& operator<<(std::ostream &out, const matrix<T, ROWS, COLS> &m)
{
    m.print(out);
    return out;
}

template<typename T, int ROWS, int COLS>
matrix<T, ROWS, COLS> operator-(const matrix<T, ROWS, COLS> &a)
{
    matrix<T, ROWS, COLS> o {};
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            o(i, j) = -a(i, j);
        }
    }    
    return o;
}

template<typename T, int ROWS, int COLS>
matrix<T, ROWS, COLS>& operator+=(matrix<T, ROWS, COLS> &a, const matrix<T, ROWS, COLS> &b)
{
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            a(i, j) += b(i, j);
        }
    }

    return a;
}

template<typename T, int ROWS, int COLS>
matrix<T, ROWS, COLS>& operator-=(matrix<T, ROWS, COLS> &a, const matrix<T, ROWS, COLS> &b)
{
    return a += -b;
}

template<typename T, int ROWS, int COLS>
matrix<T, ROWS, COLS>& operator*=(matrix<T, ROWS, COLS> &m, double c)
{
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            m(i, j) *= c;
        }
    }
    return m;
}

template<typename T, int ROWS, int COLS>
matrix<T, ROWS, COLS>& operator/=(matrix<T, ROWS, COLS> &m, double c)
{
    return m *= (1.0/c);    
}

template<typename T, int ROWS, int COLS>
matrix<T, ROWS, COLS> operator+(const matrix<T, ROWS, COLS>& a, 
                                const matrix<T, ROWS, COLS>& b)
{
    matrix<T, ROWS, COLS> o {};
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            o(i, j) = a(i, j) + b(i, j);
        }   
    }
    return o;
}

template<typename T, int ROWS, int COLS>
matrix<T, ROWS, COLS> operator-(const matrix<T, ROWS, COLS>& a, 
                                const matrix<T, ROWS, COLS>& b)
{
    return a + -b;
}

template<typename T, int ROWS, int COLS>
matrix<T, ROWS, COLS> operator*(const matrix<T, ROWS, COLS>& m, double c)
{
    matrix<T, ROWS, COLS> o {};
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            o(i, j) = m(i, j) * c;
        }
    }
    return o;
}

template<typename T, int ROWS, int COLS>
matrix<T, ROWS, COLS> operator*(double c, const matrix<T, ROWS, COLS>& m)
{
    return m*c;
}

template<typename T, int ROWS, int COLS>
matrix<T, ROWS, COLS> operator/(const matrix<T, ROWS, COLS>& m, double c)
{
    return (1.0/c) * m;   
}

template<typename T, int ROWS, int COLS> 
bool operator==(const matrix<T, ROWS, COLS> &a, 
		        const matrix<T, ROWS, COLS> &b)
{
	bool eq = true;
	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLS; j++)
		{
			if (a(i, j) != b(i, j))
				return false;
		}
	}
	return eq;
}

template<typename T, int ROWS, int COLS> 
bool operator!=(const matrix<T, ROWS, COLS> &a, 
		        const matrix<T, ROWS, COLS> &b)
{
	return !(a == b);
}

template<typename T, int ROWS, int COLS, int oROWS, int oCOLS>
void msubmatrix(const matrix<T, ROWS, COLS> &m,
                matrix<T, oROWS, oCOLS> &o,
                const std::array<int, oROWS> &rv,
                const std::array<int, oCOLS> &cv)
{    
    int oi = 0;
    for (const int &i : rv)
    {
        int oj = 0;
        for (const int &j : cv)
        {
            o(oi, oj) = m(i, j);
            oj++;
        }
        oi++;
    }
    return;
}

template<typename T, int ROWS, int COLS, int oROWS, int oCOLS>
void msubmatrix(const matrix<T, ROWS, COLS> &m,
                matrix<T, oROWS, oCOLS> &o,
                const int startrow, 
                const int startcol) 
{    
    assert(startrow + oROWS <= m.get_rows() && "msubmatrix(): Given row range out of bounds");
    assert(startcol + oCOLS <= m.get_cols() && "msubmatrix(): Given column range out of bounds");
    int oi = 0;
    for (int i = startrow; i < startrow + oROWS; i++)
    {
        int oj = 0;
        for (int j = startcol; j < startcol + oCOLS; j++)
        {
            o(oi, oj) = m(i, j);
            oj++;
        }
        oi++;
    }
    return;
}

template <typename T, int aROWS, int aCOLS, int bCOLS>
matrix<T, aROWS, bCOLS> mmult(const matrix<T, aROWS, aCOLS> &a,
		                   const matrix<T, aCOLS, bCOLS> &b)
{
	matrix<T, aROWS, bCOLS> o {};
	for (int j = 0; j < bCOLS; j++) 
	{
		for (int i = 0; i < aROWS; i++) 
		{
			T acc = 0.0;
			for (int k = 0; k < aCOLS; k++) 
			{
				acc += a(i, k) * b(k, j);
			}
			o(i, j) = acc;
		}
	}
	return o;
}

template<typename T, int ROWS, int COLS>
matrix<T, ROWS, COLS> matrix<T, ROWS, COLS>::rref() const
{
    matrix<T, ROWS, COLS> outm = (*this).ref();
    
    for (int i = get_row_bound()-1; i >= 0; i--) 
    {
        for (int j = 0; j < get_col_bound(); j++) {
            T pivot;
            if ((pivot = outm(i, j)) != 0.0)
            {
                // Normalize pivot to 1.0
                T mult = 1.0/pivot;
                outm.multRow(i, mult);
                // Zero values above pivot
                for (int i2 = i-1; i2 >= 0; i2--) 
                {
                    T mult = outm(i2, j);
                    outm.replaceRow(i2, i, -mult);
                }
                break;
            }
        }
    }
    return outm;
}

template<typename T, int ROWS, int COLS>
matrix<T, ROWS, COLS> matrix<T, ROWS, COLS>::ref() const
{
    matrix<T, ROWS, COLS> outm { (*this) };

    int pivrow = -1;
    int pivcol = -1;
    T pivot = 0;
    T mult = 0;

    int swaprow = 0;

    while(outm.get_pivot(pivrow + 1, pivcol + 1, pivrow, pivcol) > -1)
    {
        outm.swapRows(pivrow, swaprow);
        pivrow = swaprow;
        swaprow++;
        pivot = outm(pivrow, pivcol);

        for (int i = pivrow + 1; i < get_row_bound(); i++)
        {
            mult = outm(i, pivcol) / pivot;
            outm.replaceRow(i, pivrow, -mult);
        }
    }

    return outm;
}

template<typename T, int ROWS, int COLS>
int matrix<T, ROWS, COLS>::get_pivot(int starting_row, int starting_col, int &row, int &col) const
{
    if (starting_row > get_row_bound() || starting_col > get_col_bound())
        return -1;

    T abspiv = 0;
    T next = 0;
    for (int j = starting_col; j < get_col_bound(); j++)
    {
        for (int i = starting_row; i < get_row_bound(); i++)
        {
            next = std::abs((*this)(i, j));
            if (next > abspiv) 
            {
                abspiv = next;
                row = i;
                col = j;
            }
        }
        if (abspiv > 0.0)
            return 0;
    }
    return -1;
}

template<typename T, int ROWS, int COLS>
bool matrix<T, ROWS, COLS>::is_identity() const
{
    if (get_row_bound() != get_col_bound())
       return false;

    for (int i = 0; i < get_row_bound(); i++)
    {
        for (int j = 0; j < get_col_bound(); j++)
        {
            if (i != j && (*this)(i, j) != 0)
                return false;
            if (i == j && (*this)(i, j) != 1.0)
                return false;
        } 
    }
    return true;
}

template<typename T, int ROWS, int COLS>
void zerocheck(matrix<T, ROWS, COLS> &m)
{
	T bound = 1.0e-15; 
	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLS; j++) 
		{
			T val = std::abs(m(i, j));	
			if (val < bound)
				m(i, j) = 0;
		}
	}
}

template<typename T, int ROWS, int COLS>
void matrix<T, ROWS, COLS>::replaceRow(int r1, int r2, T n)
{
    auto nr = (*this)(r1) + (*this)(r2)*n;
	zerocheck(nr); // Rounding anything close to zero to zero here to make row reduction easier
    set_row(r1, nr);
}

template<typename T, int ROWS, int COLS>
void matrix<T, ROWS, COLS>::swapRows(int r1, int r2)
{
    auto tr = (*this)(r1);
    set_row(r1, (*this)(r2));
    set_row(r2, tr);
}

template<typename T, int ROWS, int COLS>
void matrix<T, ROWS, COLS>::multRow(int r, T n)
{
    auto mr = (*this)(r)*n;
	zerocheck(mr); // Rounding anything close to zero to zero here to make row reduction easier
    set_row(r, mr);
}



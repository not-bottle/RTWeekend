#if !defined(MATRIX_H)
#define MATRIX_H

#include <array>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using typeReal = double;

template<typename T, int ROWS, int COLS>
class matrix
{
private:
    int rows = ROWS;
    int cols = COLS;

    std::string template_string() const;

    int get_pivot(int starting_row, int starting_col, int &row, int &col)const;
    void replaceRow(int r1, int r2, T n);
    void swapRows(int r1, int r2);
    void multRow(int r, T n);
protected:
    std::array<T, ROWS * COLS> e {};
public:
    matrix();
    matrix(T x);
    matrix(const std::array<T, ROWS*COLS> arr);
    ~matrix() = default;

    int get_row_bound() const;
    int get_col_bound() const;
    int get_rows() const;
    int get_cols() const;
    
    void set_row(int i, matrix<T, 1, COLS> r);

    bool is_identity() const;

    matrix<T, COLS, ROWS> transpose() const;
    matrix<T, ROWS, COLS> inverse() const;

    void print(std::ostream &out) const;   

    T operator()(int i, int j) const;
    T& operator()(int i, int j);
    matrix<T, 1, COLS> operator()(int i) const;

    matrix<T, ROWS, COLS> ref() const;

    matrix<T, ROWS, COLS> rref() const;

};

/* Operator Overloads */
template<typename T, int ROWS, int COLS> 
std::ostream& operator<<(std::ostream &out, const matrix<T, ROWS, COLS> &m);

template<typename T, int ROWS, int COLS> 
matrix<T, ROWS, COLS> operator-(const matrix<T, ROWS, COLS>& a);

template<typename T, int ROWS, int COLS> 
matrix<T, ROWS, COLS>& operator*=(const matrix<T, ROWS, COLS> &a, double c);

template<typename T, int ROWS, int COLS> 
matrix<T, ROWS, COLS>& operator/=(const matrix<T, ROWS, COLS> &a, double c);

template<typename T, int ROWS, int COLS> 
matrix<T, ROWS, COLS>& operator+=(const matrix<T, ROWS, COLS> &a, 
                                  const matrix<T, ROWS, COLS> &b);

template<typename T, int ROWS, int COLS> 
matrix<T, ROWS, COLS>& operator-=(const matrix<T, ROWS, COLS> &a, 
                                  const matrix<T, ROWS, COLS> &b);

template<typename T, int ROWS, int COLS> 
matrix<T, ROWS, COLS> operator*(const matrix<T, ROWS, COLS> &m, double c);

template<typename T, int ROWS, int COLS> 
matrix<T, ROWS, COLS> operator*(double c, const matrix<T, ROWS, COLS> &m);

template<typename T, int ROWS, int COLS> 
matrix<T, ROWS, COLS> operator/(const matrix<T, ROWS, COLS> &m, double c);

template<typename T, int ROWS, int COLS> 
matrix<T, ROWS, COLS> operator+(const matrix<T, ROWS, COLS> &a, 
                                const matrix<T, ROWS, COLS> &b);

template<typename T, int ROWS, int COLS> 
matrix<T, ROWS, COLS> operator-(const matrix<T, ROWS, COLS> &a, 
                                const matrix<T, ROWS, COLS> &b);

template<typename T, int ROWS, int COLS> 
bool operator==(const matrix<T, ROWS, COLS> &a, 
                const matrix<T, ROWS, COLS> &b);

template<typename T, int ROWS, int COLS> 
bool operator!=(const matrix<T, ROWS, COLS> &a, 
                const matrix<T, ROWS, COLS> &b);

/* Matrix Operations */

template <typename T, int aROWS, int aCOLS, int bCOLS>
matrix<T, aROWS, bCOLS> mmult(const matrix<T, aROWS, aCOLS> &a,
                           const matrix<T, aCOLS, bCOLS> &b);

#include "matrix.cpp"

#endif // MATRIX_H

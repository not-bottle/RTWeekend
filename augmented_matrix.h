#if !defined(AUGMENTED_MATRIX_H)
#define AUGMENTED_MATRIX_H

#include "matrix.h"

template<typename T, int ROWS, int COLS>
class augmented : public matrix<T, ROWS, COLS>
{
private:
    int row_a = 0;
    int col_a = 0;
    int row_b = ROWS;
    int col_b = COLS;

public:
    using matrix<T, ROWS, COLS>::matrix;

    int get_row_bound() const;
    int get_col_bound() const;

    void set_bounds(int minrow, int maxrow, int mincol, int maxcol);
};
/* Overrides */
template <typename T, int ROWS, int COLS>
int augmented<T, ROWS, COLS>::get_row_bound() const { return row_b; }
template <typename T, int ROWS, int COLS>
int augmented<T, ROWS, COLS>::get_col_bound() const { return col_b; }

/* New functions */
template<typename T, int ROWS, int COLS>
void augmented<T, ROWS, COLS>::set_bounds(int minrow, int maxrow, int mincol, int maxcol)
{
    row_a = (minrow >= 0) ? minrow : 0;
    row_b = (maxrow < ROWS && maxrow >=0) ? maxrow : ROWS;
    col_a = (mincol >= 0) ? mincol : 0;
    col_b = (maxcol < COLS && maxcol >=0) ? maxcol : COLS;
}
#endif // AUGMENTED_MATRIX_H

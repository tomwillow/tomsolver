#include "linear.h"

#include "config.h"
#include "error_type.h"

#include <cassert>
#include <vector>

namespace tomsolver {

Vec SolveLinear(Mat A, Vec b) {
    int rows = A.Rows(); // 行数
    int cols = rows;     // 列数=未知数个数

    int RankA = rows, RankAb = rows; // 初始值

    assert(rows == b.Rows()); // A行数不等于b行数

    Vec ret(rows);

    if (rows > 0) {
        cols = A.Cols();
    }
    if (cols != rows) // 不是方阵
    {
        if (rows > cols) {
            // 过定义方程组
            throw MathError(ErrorType::ERROR_OVER_DETERMINED_EQUATIONS);
        } else {
            // 不定方程组
            ret.Resize(cols);
        }
    }

    std::vector<int> TrueRowNumber(cols);

    // 列主元消元法
    for (auto y = 0, x = 0; y < rows && x < cols; y++, x++) {
        // if (A[i].size() != rows)

        // 从当前行(y)到最后一行(rows-1)中，找出x列最大的一行与y行交换
        int maxAbsRowIndex = GetMaxAbsRowIndex(A, y, rows - 1, x);
        A.SwapRow(y, maxAbsRowIndex);
        b.SwapRow(y, maxAbsRowIndex);

        while (std::abs(A.Value(y, x)) < Config::Get().epsilon) // 如果当前值为0  x一直递增到非0
        {
            x++;
            if (x == cols) {
                break;
            }

            // 交换本行与最大行
            maxAbsRowIndex = GetMaxAbsRowIndex(A, y, rows - 1, x);
            A.SwapRow(y, maxAbsRowIndex);
            b.SwapRow(y, maxAbsRowIndex);
        }

        if (x != cols && x > y) {
            TrueRowNumber[y] = x; // 补齐方程时 当前行应换到x行
        }

        if (x == cols) // 本行全为0
        {
            RankA = y;
            if (std::abs(b[y]) < Config::Get().epsilon) {
                RankAb = y;
            }

            if (RankA != RankAb) {
                // 奇异，且系数矩阵及增广矩阵秩不相等->无解
                throw MathError(ErrorType::ERROR_SINGULAR_MATRIX);
            } else {
                // 跳出for，得到特解
                break;
            }
        }

        // 主对角线化为1
        auto ratioY = A.Value(y, x);
        // y行第j个->第cols个
        std::valarray rowY = std::as_const(A).Row(y, x) / ratioY;
        A.Row(y, x) = rowY;
        b[y] /= ratioY;

        // 每行化为0
        for (auto row = y + 1; row < rows; row++) // 下1行->最后1行
        {
            if (auto ratioRow = A.Value(row, x); std::abs(ratioRow) >= Config::Get().epsilon) {
                A.Row(row, x) -= rowY * ratioRow;
                b[row] -= b[y] * ratioRow;
            }
        }
    }

    bool bIndeterminateEquation = false; // 设置此变量是因为后面rows将=cols，标记以判断是否为不定方程组

    // 若为不定方程组，空缺行全填0继续运算
    if (rows != cols) {
        A.Resize(cols, cols);
        b.Resize(cols);
        rows = cols;
        bIndeterminateEquation = true;

        // 调整顺序
        for (int i = rows - 1; i >= 0; i--) {
            if (TrueRowNumber[i] != 0) {
                A.SwapRow(i, TrueRowNumber[i]);
                b.SwapRow(i, TrueRowNumber[i]);
            }
        }
    }

    // 后置换得到x
    for (int i = rows - 1; i >= 0; i--) // 最后1行->第1行
    {
        ret[i] = b[i] - (std::as_const(A).Row(i, i + 1) * std::as_const(ret).Col(0, i + 1)).sum();
    }

    if (RankA < cols && RankA == RankAb) {
        if (bIndeterminateEquation) {
            if (!Config::Get().allowIndeterminateEquation) {
                throw MathError(ErrorType::ERROR_INDETERMINATE_EQUATION,
                                "A = " + A.ToString() + "\nb = " + b.ToString());
            }
        } else {
            throw MathError(ErrorType::ERROR_INFINITY_SOLUTIONS);
        }
    }

    return ret;
}

} // namespace tomsolver
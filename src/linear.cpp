#include "linear.h"

#include "error_type.h"
#include "config.h"

#include <cassert>

namespace tomsolver {

void SwapRow(Mat &A, Vec &b, int i, int j) {
    if (i == j)
        return;
    std::swap(A[i], A[j]);
    std::swap(b[i], b[j]);
}

Vec SolveLinear(const Mat &AA, const Vec &bb) {
    Mat A(AA);
    Vec b(bb);

    int rows = A.Rows(); // 行数
    int cols = rows;     // 列数=未知数个数

    int RankA = rows, RankAb = rows; // 初始值

    assert(rows == b.Rows()); // A行数不等于b行数

    Vec ret(rows);

    if (rows > 0)
        cols = static_cast<int>(A[0].size());
    if (cols != rows) // 不是方阵
    {
        if (rows > cols)
            throw MathError(ErrorType::ERROR_OVER_DETERMINED_EQUATIONS, ""); // 过定义方程组
        else                                                                 // 不定方程组
            ret.Resize(cols);
    }

    std::vector<decltype(rows)> TrueRowNumber(cols);

    // 列主元消元法
    for (decltype(rows) y = 0, x = 0; y < rows && x < cols; y++, x++) {
        // if (A[i].size() != rows)

        // 从当前行(y)到最后一行(rows-1)中，找出x列最大的一行与y行交换
        int maxAbsRowIndex = GetMaxAbsRowIndex(A, y, rows - 1, x);
        A.SwapRow(y, maxAbsRowIndex);
        b.SwapRow(y, maxAbsRowIndex);

        while (std::abs(A[y][x]) < Config::get().epsilon) // 如果当前值为0  x一直递增到非0
        {
            x++;
            if (x == cols)
                break;

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
            if (std::abs(b[y]) < Config::get().epsilon)
                RankAb = y;

            if (RankA != RankAb) // 奇异，且系数矩阵及增广矩阵秩不相等->无解
                throw MathError(ErrorType::ERROR_SINGULAR_MATRIX, "");
            else
                break; // 跳出for，得到特解
        }

        // 主对角线化为1
        double m_num = A[y][x];
        for (decltype(rows) j = y; j < cols; j++) // y行第j个->第cols个
            A[y][j] /= m_num;
        b[y] /= m_num;

        // 每行化为0
        for (decltype(rows) row = y + 1; row < rows; row++) // 下1行->最后1行
        {
            if (std::abs(A[row][x]) < Config::get().epsilon)
                ;
            else {
                double mi = A[row][x];
                for (auto col = x; col < cols; col++) // row行第x个->第cols个
                {
                    A[row][col] -= A[y][col] * mi;
                }
                b[row] -= b[y] * mi;
            }
        }
    }

    bool bIndeterminateEquation = false; // 设置此变量是因为后面rows将=cols，标记以判断是否为不定方程组

    // 若为不定方程组，空缺行全填0继续运算
    if (rows != cols) {
        A.Resize(cols);                    // A改为cols行
        for (auto i = rows; i < cols; i++) // A从rows行开始每行cols个数
            A[i].resize(cols);
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
    double sum_others = 0.0;
    for (int i = rows - 1; i >= 0; i--) // 最后1行->第1行
    {
        sum_others = 0.0;
        for (decltype(rows) j = i + 1; j < rows; j++) // 本列 后的元素乘以已知x 加总
        {
            sum_others += A[i][j] * ret[j];
        }
        ret[i] = b[i] - sum_others;
    }

    if (RankA < cols && RankA == RankAb) {
        if (bIndeterminateEquation) {
            if (!Config::get().allowIndeterminateEquation)
                throw MathError(ErrorType::ERROR_INDETERMINATE_EQUATION,
                                std::string("A = ") + AA.ToString() + "\nb = " + bb.ToString());
        } else
            throw MathError(ErrorType::ERROR_INFINITY_SOLUTIONS, "");
    }

    return ret;
}

} // namespace tomsolver
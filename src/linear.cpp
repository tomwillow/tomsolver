#include "linear.h"

#include "error_type.h"
#include "config.h"

#include <cassert>

namespace tomsolver {

int GetMaxAbsRowIndex(const TMatrix &A, int rowStart, int rowEnd, int col) {
    double max = 0.0;
    int index = rowStart;
    for (int i = rowStart; i <= rowEnd; i++) {
        if (abs(A[i][col]) > max) {
            max = abs(A[i][col]);
            index = i;
        }
    }
    return index;
}

void SwapRow(TMatrix &A, TVector &b, int i, int j) {
    if (i == j)
        return;
    std::swap(A[i], A[j]);
    std::swap(b[i], b[j]);
}

TVector SolveLinear(TMatrix &A, TVector &b) {
    int m = static_cast<int>(A.size()); //行数
    int n = m;                          //列数=未知数个数

    int RankA = m, RankAb = m; //初始值

    TVector x(m);

    assert(m == b.size()); // Jacobian行数不等于Phi行数

    if (m > 0) {
        n = static_cast<int>(A[0].size());
        if (n != m) //不是方阵
        {
            if (m > n)
                throw MathError(ErrorType::ERROR_OVER_DETERMINED_EQUATIONS, ""); //过定义方程组
            else                                                                 //不定方程组
                x.resize(n);
        }
    }

    std::vector<decltype(m)> TrueRowNumber(n);

    //列主元消元法
    for (decltype(m) y = 0, x = 0; y < m && x < n; y++, x++) {
        // if (A[i].size() != m)

        //从当前行(y)到最后一行(m-1)中，找出x列最大的一行与y行交换
        SwapRow(A, b, y, GetMaxAbsRowIndex(A, y, m - 1, x));

        while (abs(A[y][x]) < GetConfig().epsilon) //如果当前值为0  x一直递增到非0
        {
            x++;
            if (x == n)
                break;

            //交换本行与最大行
            SwapRow(A, b, y, GetMaxAbsRowIndex(A, y, m - 1, x));
        }

        if (x != n && x > y) {
            TrueRowNumber[y] = x; //补齐方程时 当前行应换到x行
        }

        if (x == n) //本行全为0
        {
            RankA = y;
            if (abs(b[y]) < GetConfig().epsilon)
                RankAb = y;

            if (RankA != RankAb) //奇异，且系数矩阵及增广矩阵秩不相等->无解
                throw MathError(ErrorType::ERROR_SINGULAR_MATRIX, "");
            else
                break; //跳出for，得到特解
        }

        //主对角线化为1
        double m_num = A[y][x];
        for (decltype(m) j = y; j < n; j++) // y行第j个->第n个
            A[y][j] /= m_num;
        b[y] /= m_num;

        //每行化为0
        for (decltype(m) row = y + 1; row < m; row++) //下1行->最后1行
        {
            if (abs(A[row][x]) < GetConfig().epsilon)
                ;
            else {
                double mi = A[row][x];
                for (auto col = x; col < n; col++) // row行第x个->第n个
                {
                    A[row][col] -= A[y][col] * mi;
                }
                b[row] -= b[y] * mi;
            }
        }
    }

    bool bIndeterminateEquation = false; //设置此变量是因为后面m将=n，标记以判断是否为不定方程组

    //若为不定方程组，空缺行全填0继续运算
    if (m != n) {
        A.resize(n);                 // A改为n行
        for (auto i = m; i < n; i++) // A从m行开始每行n个数
            A[i].resize(n);
        b.resize(n);
        m = n;
        bIndeterminateEquation = true;

        //调整顺序
        for (int i = m - 1; i >= 0; i--) {
            if (TrueRowNumber[i] != 0) {
                SwapRow(A, b, i, TrueRowNumber[i]);
            }
        }
    }

    //后置换得到x
    double sum_others = 0.0;
    for (int i = m - 1; i >= 0; i--) //最后1行->第1行
    {
        sum_others = 0.0;
        for (decltype(m) j = i + 1; j < m; j++) //本列 后的元素乘以已知x 加总
        {
            sum_others += A[i][j] * x[j];
        }
        x[i] = b[i] - sum_others;
    }

    if (RankA < n && RankA == RankAb) {
        if (bIndeterminateEquation)
            throw MathError(ErrorType::ERROR_INDETERMINATE_EQUATION, "");
        else
            throw MathError(ErrorType::ERROR_INFINITY_SOLUTIONS, "");
    }
    return x;
}

} // namespace tomsolver
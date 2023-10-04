#include "matrix.h"

using namespace std;

namespace tomsolver {

void TestMatrix() {
    cout << "Test Inverse: " << endl;
    {
        Matrix A = {{1, 2}, {3, 4}};
        auto inv = A.Inverse();
        Matrix expected = {{-2, 1}, {1.5, -0.5}};
        assert(inv == expected);
    }
    {
        Matrix A = {{1, 2, 3}, {4, 5, 6}, {-2, 7, 8}};
        auto inv = A.Inverse();
        Matrix expected = {{-0.083333333333333, 0.208333333333333, -0.125000000000000},
                           {-1.833333333333333, 0.583333333333333, 0.250000000000000},
                           {1.583333333333333, -0.458333333333333, -0.125000000000000}};
        assert(inv == expected);
    }
    {
        Matrix A = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
        try {
            auto inv = A.Inverse();
            assert(0);
        } catch (enumError &e) {
            // normal
        }
    }

    cout << "Test Positive Determine: " << endl;
    {
        Matrix A = {{1, 1, 1, 1}, {1, 2, 3, 4}, {1, 3, 6, 10}, {1, 4, 10, 20}};
        assert(A.PositiveDetermine());
    }
    {
        Matrix A = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
        assert(!A.PositiveDetermine());
    }
}

double dot(const Vector &a, const Vector &b) {
    assert(a.rows == b.rows);
    size_t n = a.rows;
    double sum = 0;
    for (size_t i = 0; i < n; ++i)
        sum += a[i] * b[i];
    return sum;
}

Vector operator*(double k, const Vector &V) {
    Vector ans(V);
    for (size_t i = 0; i < ans.rows; ++i)
        ans[i] *= k;
    return ans;
}

Matrix operator*(double k, const Matrix &V) {
    Matrix ans(V);
    for (size_t i = 0; i < ans.rows; ++i)
        for (size_t j = 0; j < ans.cols; ++j)
            ans[i][j] *= k;
    return ans;
}

Vector operator*(const Matrix &A, const Vector &x) {
    assert(A.cols == x.rows);
    Vector ans(A.rows);
    for (size_t i = 0; i < A.rows; ++i) {
        double sum = 0;
        for (size_t k = 0; k < A.cols; ++k) {
            sum += A[i][k] * x[k];
        }
        ans[i] = sum;
    }
    return ans;
}

std::ostream &operator<<(std::ostream &out, const Matrix &V) {
    out << "[" << endl;
    for (auto &vec : V.data) {
        for (auto &val : vec) {
            out << val << " ";
        }
        out << ";" << endl;
    }
    out << "]";
    return out;
}

std::ostream &operator<<(std::ostream &out, const Vector &V) {
    out << "[";
    for (auto &vec : V.data)
        out << vec[0] << " ";
    out << "]";
    return out;
}

size_t GetMaxAbsRowIndex(const Matrix &A, size_t RowStart, size_t RowEnd, size_t Col) {
    double max = 0.0;
    size_t index = RowStart;
    for (size_t i = RowStart; i <= RowEnd; i++) {
        if (abs(A[i][Col]) > max) {
            max = abs(A[i][Col]);
            index = i;
        }
    }
    return index;
}

bool AllIsLessThan(const Vector &v1, const Vector &v2) {
    assert(v1.rows == v2.rows);
    for (size_t i = 0; i < v1.rows; ++i) {
        if (v1[i] > v2[i])
            return false;
    }
    return true;
}

// MADE BY TAE HWAN KIM, SHIN JAE HO
// 김태환, 신재호 제작
// If you see this documents, you can learn & understand Faster.
// 밑에 자료들을 보시면, 더욱 빠르게 배우고 이해하실 수 있으실겁니다.
// https://www.wikihow.com/Find-the-Inverse-of-a-3x3-Matrix
// https://www.wikihow.com/Find-the-Determinant-of-a-3X3-Matrix
// LAST UPDATE 2020 - 03 - 30
// 마지막 업데이트 2020 - 03 - 30
// This is my Github Profile. You can use this source whenever you want.
// 제 깃허브 페이지입니다. 언제든지 이 소스를 가져다 쓰셔도 됩니다.
// https://github.com/taehwan642
// Thanks :)
// 감사합니다 :)

void getCofactor(const Matrix &A, Matrix &temp, size_t p, size_t q, size_t n) // 여인수를 구해다주는 함수!
{
    size_t i = 0, j = 0; // n - 1 X n - 1 행렬에 넣을 x, y좌표

    for (size_t row = 0; row < n; row++) {
        for (size_t col = 0; col < n; col++) {
            if (row != p && col != q) // 하나라도 안되면 안들어감! && 이기때문
            {
                temp[i][j++] = A[row][col]; // j++ <- 후처리로 따로 풀어보면, [i][j] 한 뒤, j++한거와 똑같음

                if (j == n - 1) // 1차원 작게, 3차원이면 2차원 여인수 모음 행렬이 나오니까
                {
                    j = 0; // x좌표 초기화
                    i++;   // y좌표 ++
                }
            }
        }
    }
}

double determinant(const Matrix &A, size_t n) {
    if (n == 0)
        return 0;

    if (n == 1)
        return A[0][0];

    if (n == 2) // 계산 압축
        return ((A[0][0] * A[1][1]) - (A[1][0] * A[0][1]));

    Matrix temp(A.rows - 1, A.cols - 1); // n X n 행렬의 여인수를 담을 임시 행렬

    double D = 0; // D = 한 행렬의 Determinant값

    int sign = 1; // sign = +, -, +, -.... 형태로 지속되는 결과값에 영향을 주는 정수

    for (size_t f = 0; f < n; f++) {
        getCofactor(A, temp, 0, f, n); // 0으로 고정시킨 이유는, 수학 공식 상 Determinant (행렬식)은 n개의 열 중
                                       // 아무거나 잡아도 결과값은 모두 일치하기 때문
        D += sign * A[0][f] * determinant(temp, n - 1); // 재귀 형식으로 돌아간다. f는 n X n 중 정수 n을 향해 간다.

        sign = -sign; // +, -, +, -... 형식으로 되기 때문에 반대로 만들어준다.
    }

    return D; // 마지막엔 n X n 행렬의 Determinant를 리턴해준다.
}

void adjoint(const Matrix &A, Matrix &adj) // 딸림행렬, 수반행렬
{
    if (A.rows == 1) // 예외처리
    {
        adj[0][0] = 1;
        return;
    }

    int sign = 1;

    Matrix temp(A.rows, A.cols);

    for (size_t i = 0; i < A.rows; i++) {
        for (size_t j = 0; j < A.cols; j++) {
            getCofactor(A, temp, i, j, A.rows); // 여인수 구하기, 단 i, j값으로 되기에 temp는 항상 바뀐다.

            sign = ((i + j) % 2 == 0) ? 1 : -1; // +, -, + 형식으로 되는데, 0,0 좌표면 +, 0,1좌표면 -, 이렇게 된다.

            adj[j][i] = (sign) * (determinant(temp, A.rows - 1)); // n - 1 X n - 1 은, 언제나 각 여인수 행렬 은 여인수를
                                                                  // 따오는 행렬의 크기 - 1 이기 때문이다.
        }
    }
}

bool Matrix::operator==(const Matrix &b) {
    assert(rows == b.rows);
    assert(cols == b.cols);
    for (size_t i = 0; i < rows; ++i)
        for (size_t j = 0; j < cols; ++j)
            if (abs(data[i][j] - b[i][j]) > epsilon)
                return false;
    return true;
}

Matrix &Matrix::Zero() {
    data = vector<vector<double>>(rows, vector<double>(cols, 0));
    return *this;
}

Matrix &Matrix::Ones() {
    assert(rows == cols);
    Zero();
    for (int i = 0; i < rows; ++i)
        data[i][i] = 1;
    return *this;
}

double Matrix::Norm2() const {
    double sum = 0;
    for (auto &vec : data)
        for (auto val : vec)
            sum += val * val;
    return sum;
}

double Matrix::NormInfinity() const {
    double ans = data[0][0];
    for (auto &vec : data)
        for (auto val : vec)
            ans = std::max(ans, abs(val));
    return ans;
}

double Matrix::NormNegInfinity() const {
    double ans = data[0][0];
    for (auto &vec : data)
        for (auto val : vec)
            ans = std::min(ans, abs(val));
    return ans;
}

double Matrix::Min() const {
    double ans = data[0][0];
    for (auto &vec : data)
        for (auto val : vec)
            ans = std::min(ans, val);
    return ans;
}

bool Matrix::PositiveDetermine() const {
    assert(rows == cols);
    for (size_t i = 1; i <= rows; ++i) {
        double det = determinant(*this, i);
        if (det <= 0)
            return false;
    }
    return true;
}

Matrix Matrix::Transpose() const {
    assert(rows == cols);
    Matrix ans(*this);
    for (size_t i = 0; i < rows; ++i)
        for (size_t j = 0; j < cols; ++j) {
            ans[i][j] = data[j][i];
        }
    return ans;
}

Matrix Matrix::Inverse() const {
    assert(rows == cols);
    const Matrix &A = *this;
    size_t n = rows;
    Matrix ans(n, n);
    int det = determinant(A, n); // Determinant, 역행렬을 시킬 행렬의 행렬식을 구함

    if (det == 0) // 0일때는 예외처리 (역행렬을 구할 수 없기 때문.)
    {
        throw enumError::ERROR_SINGULAR_MATRIX;
    }

    Matrix adj(n, n); // 딸림행렬 선언

    adjoint(A, adj); // 딸림행렬 초기화

    for (size_t i = 0; i < n; i++)
        for (size_t j = 0; j < n; j++)
            ans[i][j] = adj[i][j] / float(det); // 각 행렬의 원소들을 조합해 값을 도출한다.
    // 역행렬의 공식 -> 역행렬 = 1 / 행렬식 * 딸림행렬
    // 역행렬[i][j]번째 원소 = 딸림행렬[i][j]번째 원소 / 행렬식

    return ans;
}

Matrix EachDivide(const Matrix &a, const Matrix &b) {
    assert(a.rows == b.rows);
    assert(a.cols == b.cols);
    Matrix ans(a);
    for (size_t i = 0; i < a.rows; ++i)
        for (size_t j = 0; j < a.cols; ++j) {
            ans[i][j] /= b[i][j];
        }
    return ans;
}

bool IsZero(const Vector &V, double epsilon) {
    for (auto vec : V.data)
        if (vec[0] > epsilon)
            return false;
    return true;
}

} // namespace tomsolver

std::vector<double> operator-(const std::vector<double> &A, const std::vector<double> &B) {
    assert(A.size() == B.size());
    std::vector<double> ans(A);
    size_t rows = A.size();
    for (size_t i = 0; i < rows; ++i)
        ans[i] -= B[i];
    return ans;
}

bool IsZero(const std::vector<double> &V, double epsilon) {
    for (auto val : V)
        if (val > epsilon)
            return false;
    return true;
}

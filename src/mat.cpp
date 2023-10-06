#include "mat.h"

#include "config.h"
#include "error_type.h"

#include <cassert>

using namespace std;

namespace tomsolver {

Mat::Mat(int row, int col) noexcept
    : rows(row), cols(col), data(std::vector<std::vector<double>>(row, std::vector<double>(col))) {
    assert(row > 0);
    assert(col > 0);
}

Mat::Mat(int row, int col, double initValue) noexcept
    : rows(row), cols(col), data(std::vector<std::vector<double>>(row, std::vector<double>(col, initValue))) {}

Mat::Mat(const std::vector<std::vector<double>> &init) noexcept {
    rows = static_cast<int>(init.size());
    assert(rows > 0);
    cols = static_cast<int>(init[0].size());
    assert(cols > 0);
    for (auto &vec : init) {
        assert(vec.size() == cols);
    }
    data = init;
}

Mat::Mat(std::vector<std::vector<double>> &&init) noexcept
    : rows(static_cast<int>(init.size())), cols(static_cast<int>(init.size() ? init[0].size() : 0)),
      data(std::move(init)) {}

Mat::Mat(const std::initializer_list<std::initializer_list<double>> &init) noexcept {
    rows = static_cast<int>(init.size());
    assert(rows > 0);
    cols = static_cast<int>(init.begin()->size());
    assert(cols > 0);
    for (auto &vec : init) {
        assert(vec.size() == cols);
    }
    data = std::vector<std::vector<double>>(init.begin(), init.end());
}

std::vector<double> &Mat::operator[](std::size_t i) noexcept {
    return data[i];
}

const std::vector<double> &Mat::operator[](std::size_t i) const noexcept {
    return data[i];
}

bool Mat::operator==(double m) const noexcept {
    for (auto &vec : data)
        for (auto &val : vec) {
            if (abs(val - m) >= GetConfig().epsilon)
                return false;
        }
    return true;
}

bool Mat::operator==(const Mat &b) const noexcept {
    assert(rows == b.rows);
    assert(cols == b.cols);
    for (std::size_t i = 0; i < rows; ++i)
        for (std::size_t j = 0; j < cols; ++j)
            if (abs(data[i][j] - b[i][j]) > GetConfig().epsilon)
                return false;
    return true;
}

// be negative
Mat Mat::operator-() noexcept {
    Mat ans(*this);
    for (auto &vec : ans.data)
        for (auto &val : vec)
            val = -val;
    return ans;
}

Mat Mat::operator+(const Mat &b) const noexcept {
    assert(rows == b.rows);
    assert(cols == b.cols);
    Mat ans(b);
    for (std::size_t i = 0; i < rows; ++i) {
        for (std::size_t j = 0; j < cols; ++j) {
            ans[i][j] = data[i][j] + b[i][j];
        }
    }
    return ans;
}

Mat &Mat::operator+=(const Mat &b) noexcept {
    assert(rows == b.rows);
    assert(cols == b.cols);
    for (std::size_t i = 0; i < rows; ++i)
        for (std::size_t j = 0; j < cols; ++j)
            data[i][j] += b[i][j];
    return *this;
}

Mat Mat::operator-(const Mat &b) const noexcept {
    assert(rows == b.rows);
    assert(cols == b.cols);
    Mat ans(b);
    for (std::size_t i = 0; i < rows; ++i) {
        for (std::size_t j = 0; j < cols; ++j) {
            ans[i][j] = data[i][j] - b[i][j];
        }
    }
    return ans;
}

Mat Mat::operator*(double m) const noexcept {
    Mat ans = *this;
    for (auto &vec : ans.data)
        for (auto &val : vec)
            val *= m;
    return ans;
}

Mat Mat::operator*(const Mat &b) const noexcept {
    assert(cols == b.rows);
    Mat ans(rows, b.cols);
    for (std::size_t i = 0; i < rows; ++i) {
        double sum = 0;
        for (std::size_t j = 0; j < b.cols; ++j) {
            for (std::size_t k = 0; k < cols; ++k) {
                sum += data[i][k] * b[k][j];
            }
            ans[i][j] = sum;
        }
    }
    return ans;
}

int Mat::Rows() const noexcept {
    return rows;
}

int Mat::Cols() const noexcept {
    return cols;
}

Vec Mat::ToVec() const {
    assert(rows > 0);
    if (cols != 1) {
        throw runtime_error("Mat::ToVec fail. rows is not one");
    }
    Vec v(rows);
    v.cols = 1;
    v.data = data;
    return v;
}

Mat &Mat::SwapRow(std::size_t i, std::size_t j) noexcept {
    if (i == j)
        return *this;
    assert(i >= 0);
    assert(i < rows);
    assert(j >= 0);
    assert(j < rows);

    std::swap(data[i], data[j]);
    return *this;
}

std::string Mat::ToString() const noexcept {
    if (data.empty())
        return "[]";
    std::string s;
    s.reserve(256);

    auto OutputRow = [&](int i) {
        int j = 0;
        for (; j < Cols() - 1; ++j) {
            s += tomsolver::ToString(data[i][j]) + ", ";
        }
        s += tomsolver::ToString(data[i][j]);
    };

    s += "[";
    OutputRow(0);
    s += "\n";

    int i = 1;
    for (; i < Rows() - 1; ++i) {
        s += " ";
        OutputRow(i);
        s += "\n";
    }
    s += " ";
    OutputRow(i);
    s += "]";
    return s;
}

void Mat::Resize(int newRows) noexcept {
    assert(newRows > 0);
    if (newRows < rows)
        data.resize(newRows);
    else {
        data.resize(newRows);
        for (std::size_t i = rows; i < newRows; ++i)
            data[i] = std::vector<double>(cols, 0);
    }
    rows = newRows;
}

Mat &Mat::Zero() noexcept {
    data = vector<vector<double>>(rows, vector<double>(cols, 0));
    return *this;
}

Mat &Mat::Ones() noexcept {
    assert(rows == cols);
    Zero();
    for (int i = 0; i < rows; ++i)
        data[i][i] = 1;
    return *this;
}

double Mat::Norm2() const noexcept {
    double sum = 0;
    for (auto &vec : data)
        for (auto val : vec)
            sum += val * val;
    return sum;
}

double Mat::NormInfinity() const noexcept {
    double ans = data[0][0];
    for (auto &vec : data)
        for (auto val : vec)
            ans = std::max(ans, abs(val));
    return ans;
}

double Mat::NormNegInfinity() const noexcept {
    double ans = data[0][0];
    for (auto &vec : data)
        for (auto val : vec)
            ans = std::min(ans, abs(val));
    return ans;
}

double Mat::Min() const noexcept {
    double ans = data[0][0];
    for (auto &vec : data)
        for (auto val : vec)
            ans = std::min(ans, val);
    return ans;
}

void Mat::SetValue(double value) noexcept {
    data = vector<vector<double>>(rows, vector<double>(cols, value));
}

bool Mat::PositiveDetermine() const noexcept {
    assert(rows == cols);
    for (int i = 1; i <= rows; ++i) {
        double det = Det(*this, i);
        if (det <= 0)
            return false;
    }
    return true;
}

Mat Mat::Transpose() const noexcept {
    Mat ans(cols, rows);
    for (std::size_t i = 0; i < rows; ++i)
        for (std::size_t j = 0; j < cols; ++j) {
            ans[j][i] = data[i][j];
        }
    return ans;
}

Mat Mat::Inverse() const {
    assert(rows == cols);
    const Mat &A = *this;
    int n = rows;
    Mat ans(n, n);
    double det = Det(A, n); // Determinant, 역행렬을 시킬 행렬의 행렬식을 구함

    if (std::abs(det) <= GetConfig().epsilon) // 0일때는 예외처리 (역행렬을 구할 수 없기 때문.)
    {
        throw MathError(ErrorType::ERROR_SINGULAR_MATRIX, "");
    }

    Mat adj(n, n); // 딸림행렬 선언

    Adjoint(A, adj); // 딸림행렬 초기화

    for (std::size_t i = 0; i < n; i++)
        for (std::size_t j = 0; j < n; j++)
            ans[i][j] = adj[i][j] / det; // 각 행렬의 원소들을 조합해 값을 도출한다.
    // 역행렬의 공식 -> 역행렬 = 1 / 행렬식 * 딸림행렬
    // 역행렬[i][j]번째 원소 = 딸림행렬[i][j]번째 원소 / 행렬식

    return ans;
}

Mat operator*(double k, const Mat &mat) noexcept {
    Mat ans(mat);
    for (std::size_t i = 0; i < ans.rows; ++i)
        for (std::size_t j = 0; j < ans.cols; ++j)
            ans[i][j] *= k;
    return ans;
}

Mat EachDivide(const Mat &a, const Mat &b) noexcept {
    assert(a.rows == b.rows);
    assert(a.cols == b.cols);
    Mat ans(a);
    for (std::size_t i = 0; i < a.rows; ++i)
        for (std::size_t j = 0; j < a.cols; ++j) {
            ans[i][j] /= b[i][j];
        }
    return ans;
}

bool IsZero(const Mat &mat) noexcept {
    for (auto &vec : mat.data)
        for (auto d : vec)
            if (d > GetConfig().epsilon)
                return false;
    return true;
}

bool AllIsLessThan(const Mat &v1, const Mat &v2) noexcept {
    assert(v1.rows == v2.rows && v1.cols == v2.cols);
    for (std::size_t i = 0; i < v1.rows; ++i) {
        for (std::size_t j = 0; j < v1.cols; ++j) {
            if (v1[i][j] > v2[i][j])
                return false;
        }
    }
    return true;
}

std::size_t GetMaxAbsRowIndex(const Mat &A, std::size_t RowStart, std::size_t RowEnd, std::size_t Col) noexcept {
    double max = 0.0;
    std::size_t index = RowStart;
    for (std::size_t i = RowStart; i <= RowEnd; i++) {
        if (abs(A[i][Col]) > max) {
            max = abs(A[i][Col]);
            index = i;
        }
    }
    return index;
}

void Adjoint(const Mat &A, Mat &adj) noexcept // 딸림행렬, 수반행렬
{
    if (A.rows == 1) // 예외처리
    {
        adj[0][0] = 1;
        return;
    }

    int sign = 1;

    Mat temp(A.rows, A.cols);

    for (std::size_t i = 0; i < A.rows; i++) {
        for (std::size_t j = 0; j < A.cols; j++) {
            GetCofactor(A, temp, i, j, A.rows); // 여인수 구하기, 단 i, j값으로 되기에 temp는 항상 바뀐다.

            sign = ((i + j) % 2 == 0) ? 1 : -1; // +, -, + 형식으로 되는데, 0,0 좌표면 +, 0,1좌표면 -, 이렇게 된다.

            adj[j][i] = (sign) * (Det(temp, A.rows - 1)); // n - 1 X n - 1 은, 언제나 각 여인수 행렬 은 여인수를
                                                          // 따오는 행렬의 크기 - 1 이기 때문이다.
        }
    }
}

void GetCofactor(const Mat &A, Mat &temp, std::size_t p, std::size_t q,
                 std::size_t n) noexcept // 여인수를 구해다주는 함수!
{
    std::size_t i = 0, j = 0; // n - 1 X n - 1 행렬에 넣을 x, y좌표

    for (std::size_t row = 0; row < n; row++) {
        for (std::size_t col = 0; col < n; col++) {
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

double Det(const Mat &A, int n) noexcept {
    if (n == 0)
        return 0;

    if (n == 1)
        return A[0][0];

    if (n == 2) // 계산 압축
        return ((A[0][0] * A[1][1]) - (A[1][0] * A[0][1]));

    Mat temp(A.rows - 1, A.cols - 1); // n X n 행렬의 여인수를 담을 임시 행렬

    double D = 0; // D = 한 행렬의 Determinant값

    int sign = 1; // sign = +, -, +, -.... 형태로 지속되는 결과값에 영향을 주는 정수

    for (std::size_t f = 0; f < n; f++) {
        GetCofactor(A, temp, 0, f, n); // 0으로 고정시킨 이유는, 수학 공식 상 Determinant (행렬식)은 n개의 열 중
                                       // 아무거나 잡아도 결과값은 모두 일치하기 때문
        D += sign * A[0][f] * Det(temp, n - 1); // 재귀 형식으로 돌아간다. f는 n X n 중 정수 n을 향해 간다.

        sign = -sign; // +, -, +, -... 형식으로 되기 때문에 반대로 만들어준다.
    }

    return D; // 마지막엔 n X n 행렬의 Determinant를 리턴해준다.
}

Vec::Vec(int rows) noexcept : Mat(rows, 1) {}

Vec::Vec(int rows, double initValue) noexcept : Mat(rows, 1, initValue) {}

Vec::Vec(const std::initializer_list<double> &init) noexcept : Vec(static_cast<int>(init.size())) {
    data.resize(rows, std::vector<double>(1));
    std::size_t i = 0;
    for (auto v : init)
        data[i++][0] = v;
}

Mat &Vec::AsMat() noexcept {
    return *this;
}

void Vec::Resize(int newRows) noexcept {
    assert(newRows > 0);
    data.resize(newRows, std::vector<double>(1));
    rows = newRows;
}

double Dot(const Vec &a, const Vec &b) noexcept {
    assert(a.rows == b.rows);
    std::size_t n = a.rows;
    double sum = 0;
    for (std::size_t i = 0; i < n; ++i)
        sum += a[i] * b[i];
    return sum;
}

Vec operator*(double k, const Vec &v) {
    Vec ans(v);
    for (std::size_t i = 0; i < ans.rows; ++i)
        ans[i] *= k;
    return ans;
}

std::ostream &operator<<(std::ostream &out, const Mat &mat) noexcept {
    return out << mat.ToString();
}

std::vector<double> operator-(const std::vector<double> &A, const std::vector<double> &B) {
    assert(A.size() == B.size());
    std::vector<double> ans(A);
    std::size_t rows = A.size();
    for (std::size_t i = 0; i < rows; ++i)
        ans[i] -= B[i];
    return ans;
}

} // namespace tomsolver
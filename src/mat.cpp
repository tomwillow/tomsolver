#include "mat.h"

#include "config.h"
#include "error_type.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <iterator>
#include <sstream>
#include <tuple>
#include <valarray>

namespace tomsolver {

Mat::Mat(int rows, int cols, double initValue) noexcept : rows(rows), cols(cols), data(initValue, rows * cols) {
    assert(rows > 0);
    assert(cols > 0);
}

Mat::Mat(std::initializer_list<std::initializer_list<double>> init) noexcept {
    rows = static_cast<int>(init.size());
    assert(rows > 0);
    cols = static_cast<int>(std::max(init, [](auto lhs, auto rhs) {
                                return lhs.size() < rhs.size();
                            }).size());
    assert(cols > 0);
    data.resize(rows * cols);

    auto i = 0;
    for (auto values : init) {
        Row(i++) = values;
    }
}

Mat::Mat(int rows, int cols, std::valarray<double> data) noexcept : rows(rows), cols(cols), data(std::move(data)) {}

std::slice_array<double> Mat::Row(int i, int offset) {
    return data[std::slice(cols * i + offset, cols - offset, 1)];
}

std::slice_array<double> Mat::Col(int j, int offset) {
    return data[std::slice(j + offset * cols, rows - offset, cols)];
}

auto Mat::Row(int i, int offset) const -> decltype(std::declval<const std::valarray<double>>()[std::slice{}]) {
    return data[std::slice(cols * i + offset, cols - offset, 1)];
}

auto Mat::Col(int j, int offset) const -> decltype(std::declval<const std::valarray<double>>()[std::slice{}]) {
    return data[std::slice(j + offset * cols, rows - offset, cols)];
}

const double &Mat::Value(int i, int j) const {
    return data[i * cols + j];
}

double &Mat::Value(int i, int j) {
    return data[i * cols + j];
}

bool Mat::operator==(double m) const noexcept {
    return std::all_of(std::begin(data), std::end(data), [m](auto val) {
        return std::abs(val - m) < Config::get().epsilon;
    });
}

bool Mat::operator==(const Mat &b) const noexcept {
    assert(rows == b.rows);
    assert(cols == b.cols);
    return std::all_of(std::begin(data), std::end(data), [iter = std::begin(b.data)](auto val) mutable {
        return std::abs(val - *iter++) < Config::get().epsilon;
    });
}

// be negative
Mat Mat::operator-() noexcept {
    return {rows, cols, -data};
}

Mat Mat::operator+(const Mat &b) const noexcept {
    assert(rows == b.rows);
    assert(cols == b.cols);
    return {rows, cols, data + b.data};
}

Mat &Mat::operator+=(const Mat &b) noexcept {
    assert(rows == b.rows);
    assert(cols == b.cols);
    data += b.data;
    return *this;
}

Mat Mat::operator-(const Mat &b) const noexcept {
    assert(rows == b.rows);
    assert(cols == b.cols);
    return {rows, cols, data - b.data};
}

Mat Mat::operator*(double m) const noexcept {
    return {rows, cols, data * m};
}

Mat Mat::operator*(const Mat &b) const noexcept {
    assert(cols == b.rows);
    Mat ans(rows, b.cols);
    for (auto i = 0; i < rows; ++i) {
        for (auto j = 0; j < b.cols; ++j) {
            ans.Value(i, j) = (Row(i) * b.Col(j)).sum();
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
        throw std::runtime_error("Mat::ToVec fail. rows is not one");
    }
    Vec v(rows);
    v.cols = 1;
    v.data = data;
    return v;
}

Mat &Mat::SwapRow(int i, int j) noexcept {
    if (i == j) {
        return *this;
    }
    assert(i >= 0);
    assert(i < rows);
    assert(j >= 0);
    assert(j < rows);

    std::valarray temp = Row(i);
    Row(i) = Row(j);
    Row(j) = temp;

    return *this;
}

Mat &Mat::SwapCol(int i, int j) noexcept {
    if (i == j) {
        return *this;
    }
    assert(i >= 0);
    assert(i < cols);
    assert(j >= 0);
    assert(j < cols);

    std::valarray t = Col(i);
    Col(i) = Col(j);
    Col(j) = t;

    return *this;
}

std::string Mat::ToString() const noexcept {
    if (data.size() == 0) {
        return "[]";
    }

    std::stringstream ss;
    ss << "[";

    size_t i = 0;
    for (auto val : data) {
        ss << (i == 0 ? "" : " ") << tomsolver::ToString(val);
        i++;
        ss << (i % cols == 0 ? (i == data.size() ? "]" : "\n") : ", ");
    }

    return ss.str();
}

void Mat::Resize(int newRows, int newCols) noexcept {
    assert(newRows > 0 && newCols > 0);
    auto temp = std::move(data);
    data.resize(newRows * newCols);
    auto minRows = std::min<size_t>(rows, newRows);
    auto minCols = std::min<size_t>(cols, newCols);
    data[std::gslice(0, {minRows, minCols}, {static_cast<size_t>(newCols), 1})] =
        temp[std::gslice(0, {minRows, minCols}, {static_cast<size_t>(cols), 1})];
    rows = newRows;
    cols = newCols;
}

Mat &Mat::Zero() noexcept {
    data = 0;
    return *this;
}

Mat &Mat::Ones() noexcept {
    assert(rows == cols);
    Zero();
    data[std::slice(0, rows, cols + 1)] = 1;
    return *this;
}

double Mat::Norm2() const noexcept {
    return (data * data).sum();
}

double Mat::NormInfinity() const noexcept {
    return std::abs(data).max();
}

double Mat::NormNegInfinity() const noexcept {
    return std::abs(data).min();
}

double Mat::Min() const noexcept {
    return data.min();
}

void Mat::SetValue(double value) noexcept {
    data = value;
}

bool Mat::PositiveDetermine() const noexcept {
    assert(rows == cols);
    for (int i = 1; i <= rows; ++i) {
        if (Det(*this, i) <= 0) {
            return false;
        }
    }
    return true;
}

Mat Mat::Transpose() const noexcept {
    Mat ans(cols, rows);
    for (auto i = 0; i < cols; i++) {
        ans.Row(i) = Col(i);
    }
    return ans;
}

Mat Mat::Inverse() const {
    assert(rows == cols);
    int n = rows;
    double det = Det(*this, n); // Determinant, 역행렬을 시킬 행렬의 행렬식을 구함

    if (std::abs(det) <= Config::get().epsilon) // 0일때는 예외처리 (역행렬을 구할 수 없기 때문.)
    {
        throw MathError(ErrorType::ERROR_SINGULAR_MATRIX);
    }

    Mat adj(n, n); // 딸림행렬 선언

    Adjoint(*this, adj); // 딸림행렬 초기화

    return {n, n, adj.data / det};
}

Mat operator*(double k, const Mat &mat) noexcept {
    Mat ans(mat);
    ans.data *= k;
    return ans;
}

Mat EachDivide(const Mat &a, const Mat &b) noexcept {
    assert(a.rows == b.rows);
    assert(a.cols == b.cols);
    return {a.rows, b.cols, b.data / b.data};
}

bool IsZero(const Mat &mat) noexcept {
    return std::all_of(std::begin(mat.data), std::end(mat.data), [](auto val) {
        return std::abs(val) <= Config::get().epsilon;
    });
}

bool AllIsLessThan(const Mat &v1, const Mat &v2) noexcept {
    assert(v1.rows == v2.rows && v1.cols == v2.cols);
    return std::all_of(std::begin(v1.data), std::end(v1.data), [iter = std::begin(v2.data)](auto val) mutable {
        return val < *iter++;
    });
}

int GetMaxAbsRowIndex(const Mat &A, int rowStart, int rowEnd, int col) noexcept {
    std::valarray temp = std::abs<double>(A.Col(col)[std::slice(rowStart, rowEnd - rowStart + 1, 1)]);
    auto ret = std::distance(std::begin(temp), std::find(std::begin(temp), std::end(temp), temp.max())) + rowStart;
    return static_cast<int>(ret);
}

void Adjoint(const Mat &A, Mat &adj) noexcept // 딸림행렬, 수반행렬
{
    if (A.rows == 1) // 예외처리
    {
        adj.Value(0, 0) = 1;
        return;
    }

    Mat cofactor(A.rows - 1, A.cols - 1);

    for (int i = 0; i < A.rows; i++) {
        for (int j = 0; j < A.cols; j++) {
            GetCofactor(A, cofactor, i, j, A.rows); // 여인수 구하기, 단 i, j값으로 되기에 temp는 항상 바뀐다.

            auto det = (Det(cofactor, A.rows - 1));

            if ((i + j) % 2 != 0) {
                det = -det; // +, -, + 형식으로 되는데, 0,0 좌표면 +, 0,1좌표면 -, 이렇게 된다.
            }

            adj.Value(j, i) = det; // n - 1 X n - 1 은, 언제나 각 여인수 행렬 은
                                   // 여인수를 따오는 행렬의 크기 - 1 이기 때문이다.
        }
    }
}

void GetCofactor(const Mat &A, Mat &cofactor, int p, int q,
                 int n) noexcept // 여인수를 구해다주는 함수!
{
    /*
         ┌───┄┄┄┄┄┄┄┄┬───┬┄┄┄┄┄┄┄┄───┐   size of region A = p * q
    0 -> │           │   │           │                  B = p * (n - 1 - q)
         ┆           ┆   ┆           ┆                  C = (n - 1 - p) * q
         ┆     A     ┆   ┆     B     ┆                  D = (n - 1 - p) * (n - 1 - q)
         ┆           ┆   ┆           ┆
         ┆           ┆   ┆           ┆    left top of region
         ├───┄┄┄┄┄┄┄┄┼───┼┄┄┄┄┄┄┄┄───┤   ╔════════╤════════════════╤══════════╗
    p ─> │           │   │           │   ║ region │ origin matrix  │ cofactor ║
         ├───┄┄┄┄┄┄┄┄┼───┼┄┄┄┄┄┄┄┄───┤   ╠════════╪════════════════╪══════════╣
         ┆           ┆   ┆           ┆   ║ A      │ (0, 0)         │ (0, 0)   ║
         ┆           ┆   ┆           ┆   ╟────────┼────────────────┼──────────╢
         ┆     C     ┆   ┆     D     ┆   ║ B      │ (0, q + 1)     │ (0, q)   ║
         ┆           ┆   ┆           ┆   ╟────────┼────────────────┼──────────╢
         │           │   │           │   ║ C      │ (p + 1, 0)     │ (p, 0)   ║
    n ─> └───┄┄┄┄┄┄┄┄┴───┴┄┄┄┄┄┄┄┄───┘   ╟────────┼────────────────┼──────────╢
          ^            ^            ^    ║ D      │ (p + 1, q + 1) │ (p, q)   ║
          0            q            n    ╚════════╧════════════════╧══════════╝
    */

    auto newIndex = [n = n - 1](int p, int q) -> size_t {
        return p * n + q;
    };
    auto index = [n = A.cols](int p, int q) -> size_t {
        return p * n + q;
    };
    auto makeValarray = [](int p, int q) {
        return std::valarray<size_t>{static_cast<size_t>(p), static_cast<size_t>(q)};
    };
    auto newStride = makeValarray(n - 1, 1);
    auto stride = makeValarray(A.cols, 1);

    std::array config = {
        std::tuple{makeValarray(p, q), newIndex(0, 0), index(0, 0)},
        std::tuple{makeValarray(p, n - 1 - q), newIndex(0, q), index(0, q + 1)},
        std::tuple{makeValarray(n - 1 - p, q), newIndex(p, 0), index(p + 1, 0)},
        std::tuple{makeValarray(n - 1 - p, n - 1 - q), newIndex(p, q), index(p + 1, q + 1)},
    };

    for (const auto &conf : config) {
        const auto &size = std::get<0>(conf);
        const auto &newStart = std::get<1>(conf);
        const auto &start = std::get<2>(conf);
        if (newStart < cofactor.data.size()) {
            cofactor.data[std::gslice(newStart, size, newStride)] = A.data[std::gslice(start, size, stride)];
        }
    }
}

double Det(const Mat &A, int n) noexcept {
    if (n == 0) {
        return 0;
    }

    if (n == 1) {
        return A.Value(0, 0);
    }

    if (n == 2) // 계산 압축
    {
        return A.Value(0, 0) * A.Value(1, 1) - A.Value(1, 0) * A.Value(0, 1);
    }

    Mat cofactor(n - 1, n - 1); // n X n 행렬의 여인수를 담을 임시 행렬

    double D = 0; // D = 한 행렬의 Determinant값

    int sign = 1; // sign = +, -, +, -.... 형태로 지속되는 결과값에 영향을 주는 정수

    for (int f = 0; f < n; f++) {
        GetCofactor(A, cofactor, 0, f, n); // 0으로 고정시킨 이유는, 수학 공식 상 Determinant (행렬식)은 n개의 열 중
        // 아무거나 잡아도 결과값은 모두 일치하기 때문
        auto det = Det(cofactor, n - 1);
        auto v = A.Value(0, f);
        D += sign * v * det; // 재귀 형식으로 돌아간다. f는 n X n 중 정수 n을 향해 간다.

        sign = -sign; // +, -, +, -... 형식으로 되기 때문에 반대로 만들어준다.
    }

    return D; // 마지막엔 n X n 행렬의 Determinant를 리턴해준다.
}

Vec::Vec(int rows, double initValue) noexcept : Mat(rows, 1, initValue) {}

Vec::Vec(std::initializer_list<double> init) noexcept : Vec(std::valarray<double>{init}) {}

Vec::Vec(std::valarray<double> init) noexcept : Vec(static_cast<int>(init.size())) {
    data = std::move(init);
}

Mat &Vec::AsMat() noexcept {
    return *this;
}

void Vec::Resize(int newRows) noexcept {
    assert(newRows > 0);
    Mat::Resize(newRows, 1);
}

double &Vec::operator[](std::size_t i) noexcept {
    return data[i];
}

double Vec::operator[](std::size_t i) const noexcept {
    return data[i];
}

Vec Vec::operator+(const Vec &b) const noexcept {
    assert(rows == b.rows);
    assert(cols == 1 && b.cols == 1);
    return {data + b.data};
}

Vec Vec::operator-() noexcept {
    return {-data};
}

Vec Vec::operator-(const Vec &b) const noexcept {
    assert(rows == b.rows);
    return {data - b.data};
}

Vec Vec::operator*(double m) const noexcept {
    return {data * m};
}

Vec Vec::operator*(const Vec &b) const noexcept {
    assert(rows == b.rows);
    return {data * b.data};
}

Vec Vec::operator/(const Vec &b) const noexcept {
    assert(rows == b.rows);
    return {data / b.data};
}

bool Vec::operator<(const Vec &b) noexcept {
    assert(rows == b.rows);
    return std::all_of(std::begin(data), std::end(data), [iter = std::begin(b.data)](auto val) mutable {
        return val < *iter++;
    });
}

Vec operator*(double k, const Vec &v) {
    return {v.data * k};
}

double Dot(const Vec &a, const Vec &b) noexcept {
    assert(a.rows == b.rows);
    return (a.data * b.data).sum();
}

std::ostream &operator<<(std::ostream &out, const Mat &mat) noexcept {
    return out << mat.ToString();
}

} // namespace tomsolver
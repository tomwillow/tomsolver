/*

Original Inverse(), Adjoint(), GetCofactor(), Det() is from https://github.com/taehwan642:

///////////////////////////////////////////
    MADE BY TAE HWAN KIM, SHIN JAE HO
    김태환, 신재호 제작
    If you see this documents, you can learn & understand Faster.
    밑에 자료들을 보시면, 더욱 빠르게 배우고 이해하실 수 있으실겁니다.
    https://www.wikihow.com/Find-the-Inverse-of-a-3x3-Matrix
    https://www.wikihow.com/Find-the-Determinant-of-a-3X3-Matrix
    LAST UPDATE 2020 - 03 - 30
    마지막 업데이트 2020 - 03 - 30
    This is my Github Profile. You can use this source whenever you want.
    제 깃허브 페이지입니다. 언제든지 이 소스를 가져다 쓰셔도 됩니다.
    https://github.com/taehwan642
    Thanks :)
    감사합니다 :)
///////////////////////////////////////////

*/

#pragma once

#include <cassert>
#include <iostream>
#include <utility>
#include <valarray>

namespace tomsolver {

class Vec;

class Mat {
public:
    explicit Mat(int row, int col, double initValue = 0) noexcept;

    Mat(std::initializer_list<std::initializer_list<double>> init) noexcept;

    Mat(int row, int col, std::valarray<double> data) noexcept;

    Mat(const Mat &) = default;
    Mat(Mat &&) = default;
    Mat &operator=(const Mat &) = default;
    Mat &operator=(Mat &&) = default;

    std::slice_array<double> Row(int i, int offset = 0);
    std::slice_array<double> Col(int j, int offset = 0);
    auto Row(int i, int offset = 0) const -> decltype(std::declval<const std::valarray<double>>()[std::slice{}]);
    auto Col(int j, int offset = 0) const -> decltype(std::declval<const std::valarray<double>>()[std::slice{}]);
    const double &Value(int i, int j) const;
    double &Value(int i, int j);

    bool operator==(double m) const noexcept;
    bool operator==(const Mat &b) const noexcept;

    // be negative
    Mat operator-() noexcept;

    Mat operator+(const Mat &b) const noexcept;
    Mat &operator+=(const Mat &b) noexcept;

    Mat operator-(const Mat &b) const noexcept;

    Mat operator*(double m) const noexcept;
    Mat operator*(const Mat &b) const noexcept;

    int Rows() const noexcept;

    int Cols() const noexcept;

    /**
     * 输出Vec。如果列数不为1，抛出异常。
     * @exception runtime_error 列数不为1
     */
    Vec ToVec() const;

    Mat &SwapRow(int i, int j) noexcept;
    Mat &SwapCol(int i, int j) noexcept;

    std::string ToString() const noexcept;

    void Resize(int newRows, int newCols) noexcept;

    Mat &Zero() noexcept;

    Mat &Ones() noexcept;

    double Norm2() const noexcept;

    double NormInfinity() const noexcept;

    double NormNegInfinity() const noexcept;

    double Min() const noexcept;

    void SetValue(double value) noexcept;

    /**
     * 返回矩阵是否正定。
     */
    bool PositiveDetermine() const noexcept;

    Mat Transpose() const noexcept;

    /**
     * 计算逆矩阵。
     * @exception MathError 如果是奇异矩阵，抛出异常
     */
    Mat Inverse() const;

protected:
    int rows;
    int cols;
    std::valarray<double> data;

    friend Mat operator*(double k, const Mat &mat) noexcept;
    friend std::ostream &operator<<(std::ostream &out, const Mat &mat) noexcept;
    friend Mat EachDivide(const Mat &a, const Mat &b) noexcept;
    friend bool IsZero(const Mat &mat) noexcept;
    friend bool AllIsLessThan(const Mat &v1, const Mat &v2) noexcept;
    friend void GetCofactor(const Mat &A, Mat &temp, int p, int q, int n) noexcept;
    friend void Adjoint(const Mat &A, Mat &adj) noexcept;
    friend double Det(const Mat &A, int n) noexcept;
};

Mat operator*(double k, const Mat &mat) noexcept;

std::ostream &operator<<(std::ostream &out, const Mat &mat) noexcept;

Mat EachDivide(const Mat &a, const Mat &b) noexcept;

bool IsZero(const Mat &mat) noexcept;

bool AllIsLessThan(const Mat &v1, const Mat &v2) noexcept;

int GetMaxAbsRowIndex(const Mat &A, int rowStart, int rowEnd, int col) noexcept;

/**
 * 伴随矩阵。
 */
void Adjoint(const Mat &A, Mat &adj) noexcept;

void GetCofactor(const Mat &A, Mat &temp, int p, int q, int n) noexcept;

/**
 * 计算矩阵的行列式值。
 */
double Det(const Mat &A, int n) noexcept;

class Vec : public Mat {
public:
    explicit Vec(int rows, double initValue = 0) noexcept;

    Vec(std::initializer_list<double> init) noexcept;

    Vec(std::valarray<double> data) noexcept;

    Mat &AsMat() noexcept;

    void Resize(int newRows) noexcept;

    double &operator[](std::size_t i) noexcept;

    double operator[](std::size_t i) const noexcept;

    Vec operator+(const Vec &b) const noexcept;

    // be negative
    Vec operator-() noexcept;

    Vec operator-(const Vec &b) const noexcept;

    Vec operator*(double m) const noexcept;

    Vec operator*(const Vec &b) const noexcept;

    Vec operator/(const Vec &b) const noexcept;

    bool operator<(const Vec &b) noexcept;

    friend double Dot(const Vec &a, const Vec &b) noexcept;
    friend Vec operator*(double k, const Vec &V);
};

/**
 * 向量点乘。
 */
double Dot(const Vec &a, const Vec &b) noexcept;

} // namespace tomsolver
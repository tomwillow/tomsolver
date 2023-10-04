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

#include "error_type.h"

#include <vector>
#include <iostream>
#include <cassert>

namespace tomsolver {

class Mat {
public:
    Mat(std::size_t row, std::size_t col) noexcept;

    Mat(const std::vector<std::vector<double>> &init) noexcept;
    Mat(std::vector<std::vector<double>> &&init) noexcept;

    Mat(const std::initializer_list<std::initializer_list<double>> &init) noexcept;

    std::vector<double> &operator[](std::size_t i) noexcept;

    const std::vector<double> &operator[](std::size_t i) const noexcept;

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

    Mat &SwapRow(std::size_t i, std::size_t j) noexcept;

    std::string ToString() const noexcept;

    void Resize(std::size_t newRows) noexcept;

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
    std::vector<std::vector<double>> data;

    friend Mat operator*(double k, const Mat &mat) noexcept;
    friend std::ostream &operator<<(std::ostream &out, const Mat &mat) noexcept;
    friend Mat EachDivide(const Mat &a, const Mat &b) noexcept;
    friend bool IsZero(const Mat &mat) noexcept;
    friend bool AllIsLessThan(const Mat &v1, const Mat &v2) noexcept;
    friend void GetCofactor(const Mat &A, Mat &temp, std::size_t p, std::size_t q, std::size_t n) noexcept;
    friend void Adjoint(const Mat &A, Mat &adj) noexcept;
    friend double Det(const Mat &A, std::size_t n) noexcept;
};

Mat operator*(double k, const Mat &mat) noexcept;

std::ostream &operator<<(std::ostream &out, const Mat &mat) noexcept;

Mat EachDivide(const Mat &a, const Mat &b) noexcept;

bool IsZero(const Mat &mat) noexcept;

bool AllIsLessThan(const Mat &v1, const Mat &v2) noexcept;

std::size_t GetMaxAbsRowIndex(const Mat &A, std::size_t RowStart, std::size_t RowEnd, std::size_t Col) noexcept;

/**
 * 伴随矩阵。
 */
void Adjoint(const Mat &A, Mat &adj) noexcept;

void GetCofactor(const Mat &A, Mat &temp, std::size_t p, std::size_t q, std::size_t n) noexcept;

/**
 * 计算矩阵的行列式值。
 */
double Det(const Mat &A, std::size_t n) noexcept;

class Vec : public Mat {
public:
    Vec(std::size_t rows) noexcept;

    Vec(const std::initializer_list<double> &init) noexcept;

    void Resize(std::size_t newRows) noexcept;

    double &operator[](std::size_t i) noexcept {
        return data[i][0];
    }

    double operator[](std::size_t i) const noexcept {
        return data[i][0];
    }

    Vec operator+(const Vec &b) const noexcept {
        assert(rows == b.rows);
        assert(cols == b.cols == 1);
        Vec ans(b);
        for (std::size_t i = 0; i < rows; ++i) {
            ans[i] = data[i][0] + b[i];
        }
        return ans;
    }

    // be negative
    Vec operator-() noexcept {
        Vec ans(*this);
        for (auto &vec : ans.data)
            vec[0] = -vec[0];
        return ans;
    }

    Vec operator-(const Vec &b) const noexcept {
        assert(rows == b.rows);
        Vec ans(b);
        for (std::size_t i = 0; i < rows; ++i) {
            ans[i] = data[i][0] - b[i];
        }
        return ans;
    }

    Vec operator*(double m) const noexcept {
        Vec ans = *this;
        for (auto &vec : ans.data)
            vec[0] *= m;
        return ans;
    }

    Vec operator*(const Vec &b) const noexcept {
        assert(rows == b.rows);
        Vec ans(b);
        for (std::size_t i = 0; i < rows; ++i) {
            ans[i] = data[i][0] * b[i];
        }
        return ans;
    }

    Vec operator/(const Vec &b) const noexcept {
        assert(rows == b.rows);
        Vec ans(b);
        for (std::size_t i = 0; i < rows; ++i) {
            ans[i] = data[i][0] / b[i];
        }
        return ans;
    }

    bool operator<(const Vec &b) noexcept {
        assert(rows == b.rows);
        for (std::size_t i = 0; i < rows; ++i)
            if (data[i][0] >= b[i])
                return false;
        return true;
    }

    friend double Dot(const Vec &a, const Vec &b) noexcept;
    friend Vec operator*(double k, const Vec &V);
};

/**
 * 向量点乘。
 */
double Dot(const Vec &a, const Vec &b) noexcept;

} // namespace tomsolver
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

class Matrix {
public:
    Matrix(std::size_t row, std::size_t col) noexcept;

    Matrix(const std::vector<std::vector<double>> &init) noexcept;

    Matrix(const std::initializer_list<std::initializer_list<double>> &init) noexcept;

    std::vector<double> &operator[](std::size_t i) noexcept;

    const std::vector<double> &operator[](std::size_t i) const noexcept;

    bool operator==(double m) const noexcept;
    bool operator==(const Matrix &b) const noexcept;

    // be negative
    Matrix operator-() noexcept;

    Matrix operator+(const Matrix &b) const noexcept;
    Matrix &operator+=(const Matrix &b) noexcept;

    Matrix operator-(const Matrix &b) const noexcept;

    Matrix operator*(double m) const noexcept;
    Matrix operator*(const Matrix &b) const noexcept;

    Matrix &SwapRow(std::size_t i, std::size_t j) noexcept;

    void Resize(std::size_t newRows) noexcept;

    Matrix &Zero() noexcept;

    Matrix &Ones() noexcept;

    double Norm2() const noexcept;

    double NormInfinity() const noexcept;

    double NormNegInfinity() const noexcept;

    double Min() const noexcept;

    void SetValue(double value) noexcept;

    /**
     * 返回矩阵是否正定。
     */
    bool PositiveDetermine() const noexcept;

    Matrix Transpose() const noexcept;

    /**
     * 计算逆矩阵。
     * @exception MathError 如果是奇异矩阵，抛出异常
     */
    Matrix Inverse() const;

protected:
    std::size_t rows;
    std::size_t cols;
    std::vector<std::vector<double>> data;

    friend Matrix operator*(double k, const Matrix &mat) noexcept;
    friend std::ostream &operator<<(std::ostream &out, const Matrix &mat) noexcept;
    friend Matrix EachDivide(const Matrix &a, const Matrix &b) noexcept;
    friend bool IsZero(const Matrix &mat) noexcept;
    friend bool AllIsLessThan(const Matrix &v1, const Matrix &v2) noexcept;
    friend void GetCofactor(const Matrix &A, Matrix &temp, std::size_t p, std::size_t q, std::size_t n) noexcept;
    friend void Adjoint(const Matrix &A, Matrix &adj) noexcept;
    friend double Det(const Matrix &A, std::size_t n) noexcept;
};

Matrix operator*(double k, const Matrix &mat) noexcept;

std::ostream &operator<<(std::ostream &out, const Matrix &mat) noexcept;

Matrix EachDivide(const Matrix &a, const Matrix &b) noexcept;

bool IsZero(const Matrix &mat) noexcept;

bool AllIsLessThan(const Matrix &v1, const Matrix &v2) noexcept;

std::size_t GetMaxAbsRowIndex(const Matrix &A, std::size_t RowStart, std::size_t RowEnd, std::size_t Col) noexcept;

/**
 * 伴随矩阵。
 */
void Adjoint(const Matrix &A, Matrix &adj) noexcept;

void GetCofactor(const Matrix &A, Matrix &temp, std::size_t p, std::size_t q, std::size_t n) noexcept;

/**
 * 计算矩阵的行列式值。
 */
double Det(const Matrix &A, std::size_t n) noexcept;

class Vector : public Matrix {
public:
    Vector(std::size_t rows) noexcept;

    Vector(const std::initializer_list<double> &init) noexcept;

    void resize(std::size_t new_rows) noexcept;

    double &operator[](std::size_t i) noexcept {
        return data[i][0];
    }

    double operator[](std::size_t i) const noexcept {
        return data[i][0];
    }

    Vector operator+(const Vector &b) const noexcept {
        assert(rows == b.rows);
        assert(cols == b.cols == 1);
        Vector ans(b);
        for (std::size_t i = 0; i < rows; ++i) {
            ans[i] = data[i][0] + b[i];
        }
        return ans;
    }

    // be negative
    Vector operator-() noexcept {
        Vector ans(*this);
        for (auto &vec : ans.data)
            vec[0] = -vec[0];
        return ans;
    }

    Vector operator-(const Vector &b) const noexcept {
        assert(rows == b.rows);
        Vector ans(b);
        for (std::size_t i = 0; i < rows; ++i) {
            ans[i] = data[i][0] - b[i];
        }
        return ans;
    }

    Vector operator*(double m) const noexcept {
        Vector ans = *this;
        for (auto &vec : ans.data)
            vec[0] *= m;
        return ans;
    }

    Vector operator*(const Vector &b) const noexcept {
        assert(rows == b.rows);
        Vector ans(b);
        for (std::size_t i = 0; i < rows; ++i) {
            ans[i] = data[i][0] * b[i];
        }
        return ans;
    }

    Vector operator/(const Vector &b) const noexcept {
        assert(rows == b.rows);
        Vector ans(b);
        for (std::size_t i = 0; i < rows; ++i) {
            ans[i] = data[i][0] / b[i];
        }
        return ans;
    }

    bool operator<(const Vector &b) noexcept {
        assert(rows == b.rows);
        for (std::size_t i = 0; i < rows; ++i)
            if (data[i][0] >= b[i])
                return false;
        return true;
    }

    friend double Dot(const Vector &a, const Vector &b) noexcept;
    friend Vector operator*(double k, const Vector &V);
};

/**
 * 向量点乘。
 */
double Dot(const Vector &a, const Vector &b) noexcept;

} // namespace tomsolver
#pragma once

#include <vector>

namespace tomsolver {
class Matrix {
public:
    size_t rows;
    size_t cols;
    std::vector<std::vector<double>> data;
    Matrix(size_t row, size_t col) : rows(row), cols(col) {
        assert(row > 0);
        assert(col > 0);
        data.resize(row, std::vector<double>(col, 0));
    }

    Matrix(const std::vector<std::vector<double>> &init) {
        rows = init.size();
        assert(rows > 0);
        cols = init[0].size();
        assert(cols > 0);
        for (auto &vec : init) {
            assert(vec.size() == cols);
        }
        data = std::vector<std::vector<double>>(init.begin(), init.end());
    }

    Matrix(const std::initializer_list<std::initializer_list<double>> &init) {
        rows = init.size();
        assert(rows > 0);
        cols = (*init.begin()).size();
        assert(cols > 0);
        for (auto &vec : init) {
            assert(vec.size() == cols);
        }
        data = std::vector<std::vector<double>>(init.begin(), init.end());
    }

    std::vector<double> &operator[](size_t i) {
        return data[i];
    }

    const std::vector<double> &operator[](size_t i) const {
        return data[i];
    }

    Matrix operator*(double m) const {
        Matrix ans = *this;
        for (auto &vec : ans.data)
            for (auto &val : vec)
                val *= m;
        return ans;
    }

    Matrix operator+(const Matrix &b) const {
        assert(rows == b.rows);
        assert(cols == b.cols);
        Matrix ans(b);
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                ans[i][j] = data[i][j] + b[i][j];
            }
        }
        return ans;
    }

    Matrix operator-(const Matrix &b) const {
        assert(rows == b.rows);
        assert(cols == b.cols);
        Matrix ans(b);
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                ans[i][j] = data[i][j] - b[i][j];
            }
        }
        return ans;
    }

    // be negative
    Matrix operator-() {
        Matrix ans(*this);
        for (auto &vec : ans.data)
            for (auto &val : vec)
                val = -val;
        return ans;
    }

    bool operator==(double m) const {
        for (auto &vec : data)
            for (auto &val : vec) {
                if (abs(val - m) >= epsilon)
                    return false;
            }
        return true;
    }

    Matrix operator*(const Matrix &b) const {
        assert(cols == b.rows);
        Matrix ans(rows, b.cols);
        for (size_t i = 0; i < rows; ++i) {
            double sum = 0;
            for (size_t j = 0; j < cols; ++j) {
                for (size_t k = 0; k < cols; ++k) {
                    sum += data[i][k] * b[k][j];
                }
                ans[i][j] = sum;
            }
        }
        return ans;
    }

    Matrix &operator+=(const Matrix &b) {
        assert(rows == b.rows);
        assert(cols == b.cols);
        for (size_t i = 0; i < rows; ++i)
            for (size_t j = 0; j < cols; ++j)
                data[i][j] += b[i][j];
        return *this;
    }

    bool operator==(const Matrix &b);

    Matrix &SwapRow(size_t i, size_t j) {
        if (i == j)
            return *this;
        assert(i >= 0);
        assert(i < rows);
        assert(j >= 0);
        assert(j < rows);

        std::swap(data[i], data[j]);
        return *this;
    }

    void resize(size_t new_rows) {
        assert(new_rows > 0);
        if (new_rows < rows)
            data.resize(new_rows);
        else {
            data.resize(new_rows);
            for (size_t i = rows; i < new_rows; ++i)
                data[i] = std::vector<double>(cols, 0);
        }
        rows = new_rows;
    }

    Matrix &Zero();

    Matrix &Ones();

    double Norm2() const;

    double NormInfinity() const;

    double NormNegInfinity() const;

    double Min() const;

    // double Determinant() const;

    void SetValue(double value) {
        for (auto &vec : data)
            for (auto &val : vec)
                val = value;
    }

    bool PositiveDetermine() const;

    Matrix Transpose() const;

    Matrix Inverse() const;
};

#ifdef _DEBUG
void TestMatrix();
#endif

class Vector : public Matrix {
public:
    Vector(size_t rows) : Matrix(rows, 1) {
        assert(rows > 0);
        data.resize(rows, std::vector<double>(1));
    }

    Vector(const std::initializer_list<double> &init) : Vector(init.size()) {
        data.resize(rows, std::vector<double>(1));
        size_t i = 0;
        for (auto v : init)
            data[i++][0] = v;
    }

    void resize(size_t new_rows) {
        assert(new_rows > 0);
        data.resize(new_rows, std::vector<double>(1));
        rows = new_rows;
    }

    double &operator[](size_t i) {
        return data[i][0];
    }

    double operator[](size_t i) const {
        return data[i][0];
    }

    Vector operator+(const Vector &b) const {
        assert(rows == b.rows);
        assert(cols == b.cols == 1);
        Vector ans(b);
        for (size_t i = 0; i < rows; ++i) {
            ans[i] = data[i][0] + b[i];
        }
        return ans;
    }

    // be negative
    Vector operator-() {
        Vector ans(*this);
        for (auto &vec : ans.data)
            vec[0] = -vec[0];
        return ans;
    }

    Vector operator-(const Vector &b) const {
        assert(rows == b.rows);
        Vector ans(b);
        for (size_t i = 0; i < rows; ++i) {
            ans[i] = data[i][0] - b[i];
        }
        return ans;
    }

    Vector operator*(double m) const {
        Vector ans = *this;
        for (auto &vec : ans.data)
            vec[0] *= m;
        return ans;
    }

    Vector operator*(const Vector &b) const {
        assert(rows == b.rows);
        Vector ans(b);
        for (size_t i = 0; i < rows; ++i) {
            ans[i] = data[i][0] * b[i];
        }
        return ans;
    }

    Vector operator/(const Vector &b) const {
        assert(rows == b.rows);
        Vector ans(b);
        for (size_t i = 0; i < rows; ++i) {
            ans[i] = data[i][0] / b[i];
        }
        return ans;
    }

    bool operator<(const Vector &b) {
        assert(rows == b.rows);
        for (size_t i = 0; i < rows; ++i)
            if (data[i][0] >= b[i])
                return false;
        return true;
    }
};
// end of Vector class

double dot(const Vector &a, const Vector &b);

Matrix EachDivide(const Matrix &a, const Matrix &b);

Vector operator*(double k, const Vector &V);
Matrix operator*(double k, const Matrix &V);

Vector operator*(const Matrix &A, const Vector &x);

std::ostream &operator<<(std::ostream &out, const Matrix &V);

std::ostream &operator<<(std::ostream &out, const Vector &V);

size_t GetMaxAbsRowIndex(const Matrix &A, size_t RowStart, size_t RowEnd, size_t Col);

bool AllIsLessThan(const Vector &v1, const Vector &v2);

bool IsZero(const Vector &V, double epsilon = 1.0e-6);

#ifdef _DEBUG
#endif
} // namespace tomsolver

std::vector<double> operator-(const std::vector<double> &A, const std::vector<double> &B);

bool IsZero(const std::vector<double> &V, double epsilon = 1.0e-6);
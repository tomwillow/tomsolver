#include "matrix.h"

#include "diff.h"
#include "subs.h"

namespace tomsolver {

namespace internal {} // namespace internal

Mat::Mat(int rows, int cols) noexcept {
    assert(rows > 0 && cols > 0);
    data.resize(rows);
    for (auto &row : data) {
        row.resize(cols);
    }
}

bool Mat::Empty() const noexcept {
    return data.empty();
}

int Mat::Rows() const noexcept {
    return static_cast<int>(data.size());
}

int Mat::Cols() const noexcept {
    if (Rows()) {
        return static_cast<int>(data[0].size());
    }
    return 0;
}

void Mat::Subs(const std::unordered_map<std::string, double> &varValues) noexcept {
    for (auto &row : data) {
        for (auto &node : row) {
            node = tomsolver::Subs(std::move(node), varValues);
        }
    }
}

Mat Mat::operator-(const Mat &rhs) const noexcept {
    assert(rhs.Rows() == Rows() && rhs.Cols() == Cols());
    Mat ret(Rows(), Cols());
    for (int i = 0; i < Rows(); ++i) {
        for (int j = 0; j < Cols(); ++j) {
            ret.data[i][j] = data[i][j] - rhs.data[i][j];
        }
    }
    return ret;
}

std::string Mat::ToString() const noexcept {
    if (Empty())
        return "[]";
    std::string s;
    s.reserve(256);

    auto OutputRow = [&](int i) {
        int j = 0;
        for (; j < Cols() - 1; ++j) {
            s += data[i][j]->ToString() + ", ";
        }
        s += data[i][j]->ToString();
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

Vec::Vec(const std::initializer_list<Node> &lst) noexcept : Mat(static_cast<int>(lst.size()), 1) {
    for (auto it = lst.begin(); it != lst.end(); ++it) {
        auto &node = const_cast<Node &>(*it);
        data[it - lst.begin()][0] = std::move(node);
    }
}

Node &Vec::operator[](std::size_t index) noexcept {
    return data[index][0];
}

const Node &Vec::operator[](std::size_t index) const noexcept {
    return data[index][0];
}

Mat Jacobian(const Vec &equations, const std::vector<std::string> &vars) noexcept {
    int rows = equations.Rows();
    int cols = static_cast<int>(vars.size());
    Mat ja(rows, cols);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            ja.data[i][j] = std::move(Diff(equations[i], vars[j]));
        }
    }
    return ja;
}

Mat operator-(const Mat &lhs, const Mat &rhs) noexcept {
    return Mat();
}

} // namespace tomsolver

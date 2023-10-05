#include "symmat.h"

#include "diff.h"
#include "subs.h"

namespace tomsolver {

namespace internal {} // namespace internal

SymMat::SymMat(int rows, int cols) noexcept {
    assert(rows > 0 && cols > 0);
    data.resize(rows);
    for (auto &row : data) {
        row.resize(cols);
    }
}

SymMat SymMat::Clone() const noexcept {
    SymMat ret(Rows(), Cols());
    for (int i = 0; i < Rows(); ++i) {
        for (int j = 0; j < Cols(); ++j) {
            ret.data[i][j] = tomsolver::Clone(data[i][j]);
        }
    }
    return ret;
}

bool SymMat::Empty() const noexcept {
    return data.empty();
}

int SymMat::Rows() const noexcept {
    return static_cast<int>(data.size());
}

int SymMat::Cols() const noexcept {
    if (Rows()) {
        return static_cast<int>(data[0].size());
    }
    return 0;
}

SymVec SymMat::ToSymVec() const {
    assert(Rows() > 0);
    if (Cols() != 1) {
        throw std::runtime_error("SymMat::ToSymVec fail. rows is not one");
    }
    SymVec v(Rows());
    for (int j = 0; j < Rows(); ++j) {
        v.data[j][0] = tomsolver::Clone(data[j][0]);
    }
    return v;
}

Mat SymMat::ToMat() const {
    std::vector<std::vector<double>> arr(Rows(), std::vector<double>(Cols()));
    for (int i = 0; i < Rows(); ++i) {
        for (int j = 0; j < Cols(); ++j) {
            if (data[i][j]->type != NodeType::NUMBER) {
                throw std::runtime_error("ToMat error: node is not number");
            }
            arr[i][j] = data[i][j]->value;
        }
    }
    return Mat(std::move(arr));
}

SymMat &SymMat::Calc() {
    for (auto &row : data) {
        for (auto &node : row) {
            node->Calc();
        }
    }
    return *this;
}

SymMat &SymMat::Subs(const std::unordered_map<std::string, double> &varValues) noexcept {
    for (auto &row : data) {
        for (auto &node : row) {
            node = tomsolver::Subs(std::move(node), varValues);
        }
    }
    return *this;
}

SymMat SymMat::operator-(const SymMat &rhs) const noexcept {
    assert(rhs.Rows() == Rows() && rhs.Cols() == Cols());
    SymMat ret(Rows(), Cols());
    for (int i = 0; i < Rows(); ++i) {
        for (int j = 0; j < Cols(); ++j) {
            ret.data[i][j] = tomsolver::Clone(data[i][j]) - tomsolver::Clone(rhs.data[i][j]);
        }
    }
    return ret;
}

std::string SymMat::ToString() const noexcept {
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

SymVec::SymVec(int rows) noexcept : SymMat(rows, 1) {}

SymVec::SymVec(const std::initializer_list<Node> &lst) noexcept : SymMat(static_cast<int>(lst.size()), 1) {
    for (auto it = lst.begin(); it != lst.end(); ++it) {
        auto &node = const_cast<Node &>(*it);
        data[it - lst.begin()][0] = std::move(node);
    }
}

SymVec SymVec::operator-(const SymVec &rhs) const noexcept {
    return SymMat::operator-(rhs).ToSymVec();
}

Node &SymVec::operator[](std::size_t index) noexcept {
    return data[index][0];
}

const Node &SymVec::operator[](std::size_t index) const noexcept {
    return data[index][0];
}

SymMat Jacobian(const SymMat &equations, const std::vector<std::string> &vars) noexcept {
    int rows = equations.Rows();
    int cols = static_cast<int>(vars.size());
    SymMat ja(rows, cols);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            ja.data[i][j] = std::move(Diff(equations.data[i][0], vars[j]));
        }
    }
    return ja;
}

} // namespace tomsolver

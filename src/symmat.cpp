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

SymMat::SymMat(const std::initializer_list<std::initializer_list<Node>> &lst) noexcept
    : SymMat(static_cast<int>(lst.size()), static_cast<int>(lst.begin()->size())) {
    for (auto it1 = lst.begin(); it1 != lst.end(); ++it1) {
        std::size_t i = it1 - lst.begin();
        for (auto it2 = it1->begin(); it2 != it1->end(); ++it2) {
            std::size_t j = it2 - it1->begin();
            auto &node = const_cast<Node &>(*it2);
            data[i][j] = std::move(node);
        }
    }
}

SymMat::SymMat(const Mat &rhs) noexcept : SymMat(rhs.Rows(), rhs.Cols()) {
    for (int i = 0; i < rhs.Rows(); ++i) {
        for (int j = 0; j < rhs.Cols(); ++j) {
            data[i][j] = Num(rhs[i][j]);
        }
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

SymVec SymMat::ToSymVecOneByOne() const noexcept {
    SymVec ans(Rows() * Cols());
    int index = 0;
    for (auto &row : data) {
        for (auto &node : row) {
            ans[index++] = tomsolver::Clone(node);
        }
    }
    return ans;
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

SymMat &SymMat::Subs(const std::map<std::string, double> &varValues) noexcept {
    for (auto &row : data) {
        for (auto &node : row) {
            node = tomsolver::Subs(std::move(node), varValues);
        }
    }
    return *this;
}

SymMat &SymMat::Subs(const VarsTable &varsTable) noexcept {
    for (auto &row : data) {
        for (auto &node : row) {
            node = tomsolver::Subs(std::move(node), varsTable);
        }
    }
    return *this;
}

std::set<std::string> SymMat::GetAllVarNames() const noexcept {
    std::set<std::string> ret;
    for (int i = 0; i < Rows(); ++i) {
        for (int j = 0; j < Cols(); ++j) {
            auto varNames = data[i][j]->GetAllVarNames();
            ret.insert(varNames.begin(), varNames.end());
        }
    }
    return ret;
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

SymMat SymMat::operator*(const SymMat &rhs) const {
    if (Cols() != rhs.Rows()) {
        throw MathError(ErrorType::SIZE_NOT_MATCH, "");
    }
    SymMat ans(Rows(), rhs.Cols());
    for (int i = 0; i < Rows(); ++i) {
        for (int j = 0; j < rhs.Cols(); ++j) {
            Node sum = data[i][0] * rhs[0][j];
            for (int k = 1; k < Cols(); ++k) {
                sum += data[i][k] * rhs[k][j];
            }
            ans[i][j] = Move(sum);
        }
    }
    return ans;
}

bool SymMat::operator==(const SymMat &rhs) const noexcept {
    if (rhs.Rows() != Rows() || rhs.Cols() != Cols()) {
        return false;
    }
    for (int i = 0; i < Rows(); ++i) {
        for (int j = 0; j < Cols(); ++j) {
            if (!data[i][j]->Equal(rhs.data[i][j])) {
                return false;
            }
        }
    }
    return true;
}

const std::vector<Node> &SymMat::operator[](int row) const noexcept {
    assert(row < Rows());
    return data[row];
}

std::vector<Node> &SymMat::operator[](int row) noexcept {
    assert(row < Rows());
    return data[row];
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
            ja.data[i][j] = Diff(equations.data[i][0], vars[j]);
        }
    }
    return ja;
}

std::ostream &operator<<(std::ostream &out, const SymMat &symMat) noexcept {
    return out << symMat.ToString();
}

} // namespace tomsolver

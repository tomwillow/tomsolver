#include "symmat.h"

#include "diff.h"
#include "mat.h"
#include "node.h"
#include "subs.h"
#include <algorithm>
#include <cmath>
#include <iterator>
#include <memory>
#include <pthread.h>
#include <utility>

namespace tomsolver {

using DataType = std::valarray<Node>;

SymMat::SymMat(int rows, int cols) noexcept : rows(rows), cols(cols) {
    assert(rows > 0 && cols > 0);
    data.reset(new DataType(rows * cols));
}

SymMat::SymMat(std::initializer_list<std::initializer_list<Node>> init) noexcept {
    rows = static_cast<int>(init.size());
    cols = static_cast<int>(std::max(init, [](auto lhs, auto rhs) {
                                return lhs.size() < rhs.size();
                            }).size());
    data.reset(new DataType(rows * cols));

    auto i = 0;
    for (auto val : init) {
        auto j = 0;
        for (auto &node : val) {
            (*data)[i * cols + j++] = std::move(const_cast<Node &>(node));
        }
        i++;
    }
}

SymMat::SymMat(const Mat &rhs) noexcept : SymMat(rhs.Rows(), rhs.Cols()) {
    std::generate(std::begin(*data), std::end(*data), [p = std::addressof(rhs.Value(0, 0))]() mutable {
        return Num(*p++);
    });
}

SymMat SymMat::Clone() const noexcept {
    SymMat ret(Rows(), Cols());
    std::generate(std::begin(*ret.data), std::end(*ret.data), [iter = std::begin(*data)]() mutable {
        return tomsolver::Clone(*iter++);
    });
    return ret;
}

bool SymMat::Empty() const noexcept {
    return data->size() == 0;
}

int SymMat::Rows() const noexcept {
    return rows;
}

int SymMat::Cols() const noexcept {
    return cols;
}

SymVec SymMat::ToSymVec() const {
    assert(rows > 0);
    if (cols != 1) {
        throw std::runtime_error("SymMat::ToSymVec fail. rows is not one");
    }
    return ToSymVecOneByOne();
}

SymVec SymMat::ToSymVecOneByOne() const noexcept {
    SymVec v(rows * cols);
    std::generate(std::begin(*v.data), std::end(*v.data), [iter = std::begin(*data)]() mutable {
        return tomsolver::Clone(*iter++);
    });
    return v;
}

Mat SymMat::ToMat() const {
    std::valarray<double> newData(data->size());
    std::generate(std::begin(newData), std::end(newData), [iter = std::begin(*data)]() mutable {
        if ((**iter).type != NodeType::NUMBER) {
            throw std::runtime_error("ToMat error: node is not number");
        }
        return (**iter++).value;
    });
    return {rows, cols, newData};
}

SymMat &SymMat::Calc() {
    for (auto &node : *data) {
        node->Calc();
    }
    return *this;
}

SymMat &SymMat::Subs(const std::map<std::string, double> &varValues) noexcept {
    for (auto &node : *data) {
        node = tomsolver::Subs(std::move(node), varValues);
    }
    return *this;
}

SymMat &SymMat::Subs(const VarsTable &varsTable) noexcept {
    for (auto &node : *data) {
        node = tomsolver::Subs(std::move(node), varsTable);
    }
    return *this;
}

std::set<std::string> SymMat::GetAllVarNames() const noexcept {
    std::set<std::string> ret;
    for (auto &node : *data) {
        ret.merge(node->GetAllVarNames());
    }
    return ret;
}

SymMat SymMat::operator-(const SymMat &rhs) const noexcept {
    assert(rhs.rows == rows && rhs.cols == cols);
    SymMat ret(rows, cols);
    std::generate(std::begin(*ret.data), std::end(*ret.data),
                  [lhsIter = std::begin(*data), rhsIter = std::begin(*rhs.data)]() mutable {
                      return *lhsIter++ - *rhsIter++;
                  });
    return ret;
}

SymMat SymMat::operator*(const SymMat &rhs) const {
    if (cols != rhs.rows) {
        throw MathError(ErrorType::SIZE_NOT_MATCH);
    }

    SymMat ans(rows, rhs.cols);
    for (int i = 0; i < Rows(); ++i) {
        for (int j = 0; j < rhs.cols; ++j) {
            auto sum = Value(i, 0) * rhs.Value(0, j);
            for (int k = 1; k < cols; ++k) {
                sum += Value(i, k) * rhs.Value(k, j);
            }
            ans.Value(i, j) = Move(sum);
        }
    }
    return ans;
}

bool SymMat::operator==(const SymMat &rhs) const noexcept {
    if (rhs.rows != rows || rhs.cols != cols) {
        return false;
    }
    return std::equal(std::begin(*data), std::end(*data), std::begin(*rhs.data), [](auto &node1, auto &node2) {
        return node1->Equal(node2);
    });
}

Node &SymMat::Value(int i, int j) noexcept {
    return (*data)[i * cols + j];
}

const Node &SymMat::Value(int i, int j) const noexcept {
    return (*data)[i * cols + j];
}

std::string SymMat::ToString() const noexcept {
    if (data->size() == 0) {
        return "[]";
    }

    std::stringstream ss;
    ss << "[";

    size_t i = 0;
    for (auto &node : *data) {
        ss << (i == 0 ? "" : " ") << node->ToString();
        i++;
        ss << (i % cols == 0 ? (i == data->size() ? "]" : "\n") : ", ");
    }

    return ss.str();
}

SymVec::SymVec(int rows) noexcept : SymMat(rows, 1) {}

SymVec::SymVec(std::initializer_list<Node> init) noexcept : SymMat(static_cast<int>(init.size()), 1) {
    auto i = 0;
    for (auto &node : init) {
        (*data)[i++] = std::move(const_cast<Node &>(node));
    }
}

SymVec SymVec::operator-(const SymVec &rhs) const noexcept {
    return SymMat::operator-(rhs).ToSymVec();
}

Node &SymVec::operator[](std::size_t index) noexcept {
    return (*data)[index];
}

const Node &SymVec::operator[](std::size_t index) const noexcept {
    return (*data)[index];
}

SymMat Jacobian(const SymMat &equations, const std::vector<std::string> &vars) noexcept {
    int rows = equations.rows;
    int cols = static_cast<int>(vars.size());
    SymMat ja(rows, cols);
    std::generate(std::begin(*ja.data), std::end(*ja.data),
                  [iter = std::begin(*equations.data), &vars, i = size_t{0}]() mutable {
                      if (i == vars.size()) {
                          i = 0;
                          iter++;
                      }
                      return Diff(*iter, vars[i++]);
                  });
    return ja;
}

std::ostream &operator<<(std::ostream &out, const SymMat &symMat) noexcept {
    return out << symMat.ToString();
}

} // namespace tomsolver

#pragma once

#include "node.h"

#include <unordered_map>

namespace tomsolver {

class SymVec;
class SymMat {
public:
    /**
     *
     */
    SymMat() noexcept {}

    /**
     *
     */
    SymMat(int rows, int cols) noexcept;

    SymMat Clone() const noexcept;

    bool Empty() const noexcept;

    int Rows() const noexcept;

    int Cols() const noexcept;

    /**
     * 把矩阵的内的元素均计算为数值节点。
     * @exception runtime_error 如果有变量存在，则无法计算
     * @exception MathError 出现浮点数无效值(inf, -inf, nan)
     */
    SymMat &Calc();

    SymMat &Subs(const std::unordered_map<std::string, double> &varValues) noexcept;

    /**
     * 如果rhs和自己的维数不匹配会触发assert。
     */
    SymMat operator-(const SymMat &rhs) const noexcept;

    std::string ToString() const noexcept;

protected:
    std::vector<std::vector<Node>> data;

    friend SymMat Jacobian(const SymMat &equations, const std::vector<std::string> &vars) noexcept;
};

class SymVec : public SymMat {
public:
    /**
     * 使用初始化列表构造。注意列表内的对象将被强行移动至Vec内部。
     */
    SymVec(const std::initializer_list<Node> &lst) noexcept;

    /**
     * 如果rhs和自己的维数不匹配会触发assert。
     */
    // SymVec operator-(const SymVec &rhs) const noexcept;

    Node &operator[](std::size_t index) noexcept;

    const Node &operator[](std::size_t index) const noexcept;
};

SymMat Jacobian(const SymMat &equations, const std::vector<std::string> &vars) noexcept;

} // namespace tomsolver

#pragma once

#include "node.h"

#include <unordered_map>

namespace tomsolver {

class Vec;
class Mat {
public:
    /**
     *
     */
    Mat() noexcept {}

    /**
     *
     */
    Mat(int rows, int cols) noexcept;

    Mat Clone() const noexcept;

    bool Empty() const noexcept;

    int Rows() const noexcept;

    int Cols() const noexcept;

    /**
     * 把矩阵的内的元素均计算为数值节点。
     * @exception runtime_error 如果有变量存在，则无法计算
     * @exception MathError 出现浮点数无效值(inf, -inf, nan)
     */
    Mat &Calc();

    Mat &Subs(const std::unordered_map<std::string, double> &varValues) noexcept;

    /**
     * 如果rhs和自己的维数不匹配会触发assert。
     */
    Mat operator-(const Mat &rhs) const noexcept;

    std::string ToString() const noexcept;

protected:
    std::vector<std::vector<Node>> data;

    friend Mat Jacobian(const Mat &equations, const std::vector<std::string> &vars) noexcept;
};

class Vec : public Mat {
public:
    /**
     * 使用初始化列表构造。注意列表内的对象将被强行移动至Vec内部。
     */
    Vec(const std::initializer_list<Node> &lst) noexcept;

    /**
     * 如果rhs和自己的维数不匹配会触发assert。
     */
    // Vec operator-(const Vec &rhs) const noexcept;

    Node &operator[](std::size_t index) noexcept;

    const Node &operator[](std::size_t index) const noexcept;
};

Mat Jacobian(const Mat &equations, const std::vector<std::string> &vars) noexcept;

} // namespace tomsolver

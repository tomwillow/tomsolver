#pragma once

#include "node.h"
#include "mat.h"
#include "vars_table.h"

#include <unordered_map>

namespace tomsolver {

class SymVec;
class SymMat {
public:
    /**
     *
     */
    SymMat(int rows, int cols) noexcept;

    /**
     * 使用初始化列表构造。注意列表内的对象将被强行移动至Vec内部。
     */
    SymMat(const std::initializer_list<std::initializer_list<Node>> &lst) noexcept;

    /**
     * 从数值矩阵构造符号矩阵
     */
    SymMat(const Mat &rhs) noexcept;

    SymMat Clone() const noexcept;

    bool Empty() const noexcept;

    int Rows() const noexcept;

    int Cols() const noexcept;

    /**
     * 输出Vec。如果列数不为1，抛出异常。
     * @exception runtime_error 列数不为1
     */
    SymVec ToSymVec() const;

    /**
     * 逐个元素转换为符号向量（列向量）。
     */
    SymVec ToSymVecOneByOne() const noexcept;

    /**
     * 得到数值矩阵。前提条件是矩阵内的元素均为数值节点，否则抛出异常。
     * @exception runtime_error 存在非数值节点
     */
    Mat ToMat() const;

    /**
     * 把矩阵的内的元素均计算为数值节点。
     * @exception runtime_error 如果有变量存在，则无法计算
     * @exception MathError 出现浮点数无效值(inf, -inf, nan)
     */
    SymMat &Calc();

    SymMat &Subs(const std::unordered_map<std::string, double> &varValues) noexcept;

    SymMat &Subs(const VarsTable &varsTable) noexcept;

    /**
     * 返回符号矩阵内出现的所有变量名。
     */
    std::unordered_set<std::string> GetAllVarNames() const noexcept;

    /**
     * 如果rhs和自己的维数不匹配会触发assert。
     */
    SymMat operator-(const SymMat &rhs) const noexcept;

    /**
     *
     * @exception MathError 维数不匹配
     */
    SymMat operator*(const SymMat &rhs) const;

    /**
     * 返回是否相等。
     * 目前只能判断表达式树完全一致的情况。
     * TODO 改为可以判断等价表达式
     */
    bool operator==(const SymMat &rhs) const noexcept;

    const std::vector<Node> &operator[](int row) const noexcept;

    std::vector<Node> &operator[](int row) noexcept;

    std::string ToString() const noexcept;

protected:
    std::vector<std::vector<Node>> data;

    friend SymMat Jacobian(const SymMat &equations, const std::vector<std::string> &vars) noexcept;
};

class SymVec : public SymMat {
public:
    /**
     *
     */
    SymVec(int rows) noexcept;

    /**
     * 使用初始化列表构造。注意列表内的对象将被强行移动至Vec内部。
     */
    SymVec(const std::initializer_list<Node> &lst) noexcept;

    /**
     * 如果rhs和自己的维数不匹配会触发assert。
     */
    SymVec operator-(const SymVec &rhs) const noexcept;

    Node &operator[](std::size_t index) noexcept;

    const Node &operator[](std::size_t index) const noexcept;
};

SymMat Jacobian(const SymMat &equations, const std::vector<std::string> &vars) noexcept;

std::ostream &operator<<(std::ostream &out, const SymMat &symMat) noexcept;

} // namespace tomsolver

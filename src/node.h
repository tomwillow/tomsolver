#pragma once
#include "error_type.h"
#include "math_operator.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace tomsolver {

enum class NodeType { NUMBER, OPERATOR, VARIABLE };

// 前置声明
namespace internal {
struct NodeImpl;
}
class SymMat;

/**
 * 表达式节点。
 */
using Node = std::unique_ptr<internal::NodeImpl>;

namespace internal {

/**
 * 单个节点的实现。通常应该以std::unique_ptr包裹。
 */
struct NodeImpl {

    NodeImpl(NodeType type, MathOperator op, double value, std::string varname) noexcept
        : type(type), op(op), value(value), varname(varname), parent(nullptr) {}

    NodeImpl(const NodeImpl &rhs) noexcept;
    NodeImpl &operator=(const NodeImpl &rhs) noexcept;

    NodeImpl(NodeImpl &&rhs) noexcept;
    NodeImpl &operator=(NodeImpl &&rhs) noexcept;

    ~NodeImpl();

    bool Equal(const Node &rhs) const noexcept;

    /**
     * 把整个节点以中序遍历的顺序输出为字符串。
     * 例如：
     *      Node n = (Var("a") + Num(1)) * Var("b");
     *   则
     *      n->ToString() == "(a+1.000000)*b"
     */
    std::string ToString() const noexcept;

    /**
     * 计算出整个表达式的数值。不改变自身。
     * @exception runtime_error 如果有变量存在，则无法计算
     * @exception MathError 出现浮点数无效值(inf, -inf, nan)
     */
    double Vpa() const;

    /**
     * 计算出整个表达式的数值。不改变自身。
     * @exception runtime_error 如果有变量存在，则无法计算
     * @exception MathError 出现浮点数无效值(inf, -inf, nan)
     */
    NodeImpl &Calc();

    /**
     * 返回表达式内出现的所有变量名。
     */
    std::set<std::string> GetAllVarNames() const noexcept;

    /**
     * 检查整个节点数的parent指针是否正确。
     */
    void CheckParent() const noexcept;

private:
    NodeType type = NodeType::NUMBER;
    MathOperator op = MathOperator::MATH_NULL;
    double value;
    std::string varname;
    NodeImpl *parent = nullptr;
    Node left, right;
    NodeImpl() = default;

    /**
     * 本节点如果是OPERATOR，检查操作数数量和left, right指针是否匹配。
     */
    void CheckOperatorNum() const noexcept;

    /**
     * 节点转string。仅限本节点，不含子节点。
     */
    std::string NodeToStr() const noexcept;

    void ToStringRecursively(std::stringstream &output) const noexcept;

    void ToStringNonRecursively(std::stringstream &output) const noexcept;

    /**
     * 计算表达式数值。递归实现。
     * @exception runtime_error 如果有变量存在，则无法计算
     * @exception MathError 不符合定义域, 除0等情况。
     */
    double VpaRecursively() const;

    /**
     * 计算表达式数值。非递归实现。
     * 性能弱于递归实现。但不会导致栈溢出。
     * 根据benchmark，生成一组含4000个随机四则运算节点的表达式，生成1000次，Release下测试耗时3000ms。递归实现耗时2500ms。
     * 粗略计算，即 1333 ops/ms。
     * @exception runtime_error 如果有变量存在，则无法计算
     * @exception MathError 不符合定义域, 除0等情况。
     */
    double VpaNonRecursively() const;

    /**
     * 释放整个节点树，除了自己。
     * 实际是二叉树的非递归后序遍历。
     */
    void Release() noexcept;

    friend Node Operator(MathOperator op, Node left, Node right) noexcept;

    friend Node CloneRecursively(const Node &rhs) noexcept;
    friend Node CloneNonRecursively(const Node &rhs) noexcept;

    friend void CopyOrMoveTo(NodeImpl *parent, Node &child, Node &&n1) noexcept;
    friend void CopyOrMoveTo(NodeImpl *parent, Node &child, const Node &n1) noexcept;

    friend std::ostream &operator<<(std::ostream &out, const Node &n) noexcept;

    template <typename T>
    friend Node UnaryOperator(MathOperator op, T &&n) noexcept;

    template <typename T1, typename T2>
    friend Node BinaryOperator(MathOperator op, T1 &&n1, T2 &&n2) noexcept;

    friend class tomsolver::SymMat;
    friend class SimplifyFunctions;
    friend class DiffFunctions;
    friend class SubsFunctions;
    friend class ParseFunctions;
};

Node CloneRecursively(const Node &rhs) noexcept;

Node CloneNonRecursively(const Node &rhs) noexcept;

/**
 * 对于一个节点n和另一个节点n1，把n1移动到作为n的子节点。
 * 用法：CopyOrMoveTo(n->parent, n->left, std::forward<T>(n1));
 */
void CopyOrMoveTo(NodeImpl *parent, Node &child, Node &&n1) noexcept;

/**
 * 对于一个节点n和另一个节点n1，把n1整个拷贝一份，把拷贝的副本设为n的子节点。
 * 用法：CopyOrMoveTo(n->parent, n->left, std::forward<T>(n1));
 */
void CopyOrMoveTo(NodeImpl *parent, Node &child, const Node &n1) noexcept;

/**
 * 重载std::ostream的<<操作符以输出一个Node节点。
 */
std::ostream &operator<<(std::ostream &out, const Node &n) noexcept;

template <typename T>
Node UnaryOperator(MathOperator op, T &&n) noexcept {
    auto ret = std::make_unique<NodeImpl>(NodeType::OPERATOR, op, 0, "");
    CopyOrMoveTo(ret.get(), ret->left, std::forward<T>(n));
    return ret;
}

template <typename T1, typename T2>
Node BinaryOperator(MathOperator op, T1 &&n1, T2 &&n2) noexcept {
    auto ret = std::make_unique<NodeImpl>(NodeType::OPERATOR, op, 0, "");
    CopyOrMoveTo(ret.get(), ret->left, std::forward<T1>(n1));
    CopyOrMoveTo(ret.get(), ret->right, std::forward<T2>(n2));
    return ret;
}

/**
 * 新建一个运算符节点。
 */
Node Operator(MathOperator op, Node left = nullptr, Node right = nullptr) noexcept;

} // namespace internal

Node Clone(const Node &rhs) noexcept;

/**
 * 对节点进行移动。等同于std::move。
 */
Node Move(Node &rhs) noexcept;

/**
 * 新建一个数值节点。
 */
Node Num(double num) noexcept;

/**
 * 新建一个函数节点。
 */
Node Op(MathOperator op);

/**
 * 返回变量名是否有效。（只支持英文数字或者下划线，第一个字符必须是英文或者下划线）
 */
bool VarNameIsLegal(std::string_view varname) noexcept;

/**
 * 新建一个变量节点。
 * @exception runtime_error 名字不合法
 */
Node Var(std::string_view varname);

template <typename... T>
using SfinaeNode = std::enable_if_t<std::conjunction_v<std::is_same<std::decay_t<T>, Node>...>, Node>;

template <typename T1, typename T2>
SfinaeNode<T1, T2> operator+(T1 &&n1, T2 &&n2) noexcept {
    return internal::BinaryOperator(MathOperator::MATH_ADD, std::forward<T1>(n1), std::forward<T2>(n2));
}

template <typename T>
SfinaeNode<T> &operator+=(Node &n1, T &&n2) noexcept {
    n1 = internal::BinaryOperator(MathOperator::MATH_ADD, std::move(n1), std::forward<T>(n2));
    return n1;
}

template <typename T1, typename T2>
SfinaeNode<T1, T2> operator-(T1 &&n1, T2 &&n2) noexcept {
    return internal::BinaryOperator(MathOperator::MATH_SUB, std::forward<T1>(n1), std::forward<T2>(n2));
}

template <typename T>
SfinaeNode<T> operator-(T &&n1) noexcept {
    return internal::UnaryOperator(MathOperator::MATH_NEGATIVE, std::forward<T>(n1));
}

template <typename T>
SfinaeNode<T> operator+(T &&n1) noexcept {
    return internal::UnaryOperator(MathOperator::MATH_POSITIVE, std::forward<T>(n1));
}

template <typename T>
SfinaeNode<T> &operator-=(Node &n1, T &&n2) noexcept {
    n1 = internal::BinaryOperator(MathOperator::MATH_SUB, std::move(n1), std::forward<T>(n2));
    return n1;
}

template <typename T1, typename T2>
SfinaeNode<T1, T2> operator*(T1 &&n1, T2 &&n2) noexcept {
    return internal::BinaryOperator(MathOperator::MATH_MULTIPLY, std::forward<T1>(n1), std::forward<T2>(n2));
}

template <typename T>
SfinaeNode<T> &operator*=(Node &n1, T &&n2) noexcept {
    n1 = internal::BinaryOperator(MathOperator::MATH_MULTIPLY, std::move(n1), std::forward<T>(n2));
    return n1;
}

template <typename T1, typename T2>
SfinaeNode<T1, T2> operator/(T1 &&n1, T2 &&n2) noexcept {
    return internal::BinaryOperator(MathOperator::MATH_DIVIDE, std::forward<T1>(n1), std::forward<T2>(n2));
}

template <typename T>
SfinaeNode<T> &operator/=(Node &n1, T &&n2) noexcept {
    n1 = internal::BinaryOperator(MathOperator::MATH_DIVIDE, std::move(n1), std::forward<T>(n2));
    return n1;
}

template <typename T1, typename T2>
SfinaeNode<T1, T2> operator^(T1 &&n1, T2 &&n2) noexcept {
    return internal::BinaryOperator(MathOperator::MATH_POWER, std::forward<T1>(n1), std::forward<T2>(n2));
}

template <typename T>
SfinaeNode<T> &operator^=(Node &n1, T &&n2) noexcept {
    n1 = internal::BinaryOperator(MathOperator::MATH_POWER, std::move(n1), std::forward<T>(n2));
    return n1;
}

} // namespace tomsolver

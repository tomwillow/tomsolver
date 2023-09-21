#pragma once
#include "error_type.h"
#include "math_operator.h"

#include <iostream>

#include <vector>
#include <type_traits>
#include <memory>
#include <string>
#include <stack>
#include <cassert>

namespace tomsolver {

enum class NodeType { NUMBER, OPERATOR, VARIABLE, FUNCTION };

namespace internal {
struct NodeImpl;
}

using Node = std::unique_ptr<internal::NodeImpl>;

namespace internal {

/* 单个元素 */
struct NodeImpl {

    NodeImpl(NodeType type, MathOperator op, double value, std::string varname) noexcept
        : type(type), op(op), value(value), varname(varname), parent(nullptr) {}

    NodeImpl(const NodeImpl &rhs) noexcept {
        operator=(rhs);
    }

    NodeImpl &operator=(const NodeImpl &rhs) noexcept {
        type = rhs.type;
        op = rhs.op;
        value = rhs.value;
        varname = rhs.varname;
        parent = rhs.parent;
        left = CopyTree(rhs.left);
        right = CopyTree(rhs.right);
        return *this;
    }

    NodeImpl(NodeImpl &&rhs) noexcept {
        operator=(std::move(rhs));
    }

    NodeImpl &operator=(NodeImpl &&rhs) noexcept {
        type = rhs.type;
        op = rhs.op;
        value = rhs.value;
        varname = rhs.varname;
        parent = rhs.parent;
        left = std::move(rhs.left);
        right = std::move(rhs.right);

        rhs.parent = nullptr;
        return *this;
    }

    ~NodeImpl() {
        Release();
    }

    /**
     * 把整个节点以中序遍历的顺序输出为字符串。
     */
    std::string ToString() const noexcept {
        std::string ret;
        TraverseInOrder(ret);
        return ret;
    }

    /**
     * 迭代计算
     * @exception
     */
    double Vpa() const {
        return VpaNonRecursively();
    }

private:
    NodeType type;
    MathOperator op;
    double value;
    std::string varname;
    NodeImpl *parent;
    std::unique_ptr<NodeImpl> left, right;

    std::unique_ptr<NodeImpl> CopyTree(const std::unique_ptr<NodeImpl> &rhs) const noexcept {
        if (rhs == nullptr) {
            return nullptr;
        }
        auto ret = std::make_unique<NodeImpl>(rhs->type, rhs->op, rhs->value, rhs->varname);
        ret->left = CopyTree(rhs->left);
        ret->right = CopyTree(rhs->right);
        return ret;
    }

    /**
     * 节点转string。仅限本节点，不含子节点。
     */
    std::string NodeToStr() const noexcept {
        switch (type) {
        case NodeType::NUMBER:
            return std::to_string(value);
        case NodeType::VARIABLE:
            return varname;
        case NodeType::OPERATOR:
            return MathOperatorToStr(op);
        }
        assert(0 && "unexpected NodeType. maybe this is a bug.");
        return "";
    }

    void TraverseInOrder(std::string &output) const noexcept {
        switch (type) {
        case NodeType::NUMBER:
        case NodeType::VARIABLE:
            output += NodeToStr();
            return;
        }

        int has_parenthesis = 0;
        if (GetOperatorNum(op) == 1) //一元运算符：函数和取负
        {
            if (type == NodeType::FUNCTION) {
                output += NodeToStr() + "(";
                has_parenthesis = 1;
            } else {
                output += "(" + NodeToStr();
                has_parenthesis = 1;
            }
        } else {
            //非一元运算符才输出，即一元运算符的输出顺序已改变
            if (type == NodeType::OPERATOR) //本级为运算符
                if (parent != nullptr)
                    if ((GetOperatorNum(parent->op) == 2 && //父运算符存在，为二元，
                         (Rank(parent->op) > Rank(op)       //父级优先级高于本级->加括号

                          || ( //两级优先级相等
                                 Rank(parent->op) == Rank(op) &&
                                 (
                                     //本级为父级的右子树 且父级不满足结合律->加括号
                                     (InAssociativeLaws(parent->op) == false && this == parent->right.get()) ||
                                     //两级都是右结合
                                     (InAssociativeLaws(parent->op) == false && IsLeft2Right(op) == false)))))

                        //||

                        ////父运算符存在，为除号，且本级为分子，则添加括号
                        //(now->parent->eOperator == MATH_DIVIDE && now == now->parent->right)
                    ) {
                        output += "(";
                        has_parenthesis = 1;
                    }
        }

        if (left != nullptr) //左遍历
        {
            left->TraverseInOrder(output);
        }

        if (GetOperatorNum(op) != 1) //非一元运算符才输出，即一元运算符的输出顺序已改变
        {
            output += NodeToStr();
        }

        if (right != nullptr) //右遍历
        {
            right->TraverseInOrder(output);
        }

        //回到本级时补齐右括号，包住前面的东西
        if (has_parenthesis) {
            output += ")";
        }
    }

    
    /**
     * 计算表达式数值。递归实现。
     * @exception runtime_error 如果有变量存在，则无法计算
     * @exception MathError 不符合定义域, 除0等情况。
     */
    double VpaRecursively() const {
        double l = 0, r = 0;
        if (left != nullptr)
            l = left->Vpa();
        if (right != nullptr)
            r = right->Vpa();

        if (type == NodeType::NUMBER) {
            return value;
        }

        if (type == NodeType::VARIABLE) {
            throw std::runtime_error("has variable. can not calculate to be a number");
        }

        if (type == NodeType::OPERATOR) {
            assert((GetOperatorNum(op) == 1 && left != nullptr && right == nullptr) ||
                   (GetOperatorNum(op) == 2 && left != nullptr && right != nullptr));
            return Calc(op, l, r);
        }

        throw std::runtime_error("unsupported node type");
        return std::numeric_limits<double>::quiet_NaN();
    }

    /**
     * 计算表达式数值。非递归实现。
     * 性能弱于递归实现。但不会导致栈溢出。
     * 根据benchmark，生成一组含4000个随机四则运算节点的表达式，生成1000次，Release下测试耗时3000ms。递归实现耗时2500ms。
     * 粗略计算，即 1333 ops/ms。
     * @exception runtime_error 如果有变量存在，则无法计算
     * @exception MathError 不符合定义域, 除0等情况。
     */
    double VpaNonRecursively() const {
        std::stack<const NodeImpl*> stk;
        std::stack<const NodeImpl *> revertedPostOrder;

        // ==== Part I ====

        // 借助一个栈，得到反向的后序遍历序列，结果保存在revertedPostOrder
        stk.push(this);

        while (1) {
            if (stk.empty()) {
                break;
            }

            auto f = stk.top();
            stk.pop();

            if (f->left) {
                stk.push(f->left.get());
            }

            if (f->right) {
                stk.push(f->right.get());
            }

            revertedPostOrder.push(f);
        }

        // ==== Part II ====
        // revertedPostOrder的反向序列是一组逆波兰表达式，根据这组逆波兰表达式可以计算出表达式的值
        // calcStk是用来计算值的临时栈，计算完成后calcStk的size应该为1
        std::stack<double> calcStk;
        while (!revertedPostOrder.empty())
        {
            auto f = revertedPostOrder.top();
            revertedPostOrder.pop();

            if (f->type == NodeType::NUMBER)
            {
                calcStk.push(f->value);
                continue;
            }

            if (f->type == NodeType::OPERATOR) {
                if (GetOperatorNum(f->op) == 1) {
                    calcStk.top() = Calc(f->op, calcStk.top(), std::numeric_limits<double>::quiet_NaN());
                    continue;
                }

                if (GetOperatorNum(f->op) == 2) {
                    double r = calcStk.top();
                    calcStk.pop();

                    double &l = calcStk.top();
                    calcStk.top() = Calc(f->op, l, r);
                    continue;
                }

                assert(0 && "[VpaNonRecursively] unsupported operator num");
            }

            // 其他情况抛异常
            throw std::runtime_error("wrong");
        }

        assert(calcStk.size() == 1);

        return calcStk.top();
    }

    /**
    * 释放整个节点树，除了自己。
    * 实际是二叉树的非递归后序遍历。
    */
    void Release() noexcept {
        std::stack<Node> stk;

        if (left) {
            stk.push(std::move(left));
        }

        if (right) {
            stk.push(std::move(right));
        }

        while (1) {
            if (stk.empty()) {
                break;
            }

            auto f = std::move(stk.top());
            stk.pop();

            if (f->left) {
                stk.push(std::move(f->left));
            }

            if (f->right) {
                stk.push(std::move(f->right));
            }

            assert(f->left == nullptr && f->right == nullptr);

            // 这里如果把f填入vector，最后翻转。得到的序列就是后序遍历。
        }
    }

    friend void CopyOrMoveTo(NodeImpl *parent, std::unique_ptr<NodeImpl> &child,
                             std::unique_ptr<NodeImpl> &&n1) noexcept;
    friend void CopyOrMoveTo(NodeImpl *parent, std::unique_ptr<NodeImpl> &child,
                             const std::unique_ptr<NodeImpl> &n1) noexcept;

    friend std::ostream &operator<<(std::ostream &out, const std::unique_ptr<NodeImpl> &n) noexcept;

    template <typename T1, typename T2>
    friend std::unique_ptr<NodeImpl> OperatorSome(MathOperator op, T1 &&n1, T2 &&n2) noexcept;
};


/**
 * 对于一个节点n和另一个节点n1，把n1移动到作为n的子节点。
 */
void CopyOrMoveTo(NodeImpl *parent, std::unique_ptr<NodeImpl> &child, std::unique_ptr<NodeImpl> &&n1) noexcept;

/**
 * 对于一个节点n和另一个节点n1，把n1整个拷贝一份，把拷贝的副本设为n的子节点。
 */
void CopyOrMoveTo(NodeImpl *parent, std::unique_ptr<NodeImpl> &child, const std::unique_ptr<NodeImpl> &n1) noexcept;

/**
 * 重载std::ostream的<<操作符以输出一个Node节点。
 */
std::ostream &operator<<(std::ostream &out, const std::unique_ptr<internal::NodeImpl> &n) noexcept;

template <typename T1, typename T2>
std::unique_ptr<NodeImpl> OperatorSome(MathOperator op, T1 &&n1, T2 &&n2) noexcept {
    auto ret = std::make_unique<NodeImpl>(NodeType::OPERATOR, op, 0, "");
    CopyOrMoveTo(ret.get(), ret->left, std::forward<T1>(n1));
    CopyOrMoveTo(ret.get(), ret->right, std::forward<T2>(n2));
    return ret;
}

} // namespace internal

/**
 * 新建一个数值节点。
 */
std::unique_ptr<internal::NodeImpl> Num(double num) noexcept;

/**
 * 有效性检查（返回0则出现异常字符）
 */
// bool IsLegal(char c) {
//	if (isDoubleChar(c)) return true;
//	if (isBaseOperator(c)) return true;
//	if (IsCharAlpha(c) || c == '_') return true;
//	return false;
//}

bool VarNameIsLegal(const std::string &varname) noexcept;

/**
 * 新建一个变量节点。
 * @exception runtime_error 名字不合法
 */
std::unique_ptr<internal::NodeImpl> Var(const std::string &varname);

template <typename T1, typename T2>
std::unique_ptr<internal::NodeImpl> operator+(T1 &&n1, T2 &&n2) noexcept {
    return internal::OperatorSome(MathOperator::MATH_ADD, std::forward<T1>(n1), std::forward<T2>(n2));
}

template <typename T>
std::unique_ptr<internal::NodeImpl> &operator+=(std::unique_ptr<internal::NodeImpl> &n1, T &&n2) noexcept {
    n1 = internal::OperatorSome(MathOperator::MATH_ADD, std::move(n1), std::forward<T>(n2));
    return n1;
}

template <typename T1, typename T2>
std::unique_ptr<internal::NodeImpl> operator-(T1 &&n1, T2 &&n2) noexcept {
    return internal::OperatorSome(MathOperator::MATH_SUB, std::forward<T1>(n1), std::forward<T2>(n2));
}

template <typename T>
std::unique_ptr<internal::NodeImpl> &operator-=(std::unique_ptr<internal::NodeImpl> &n1, T &&n2) noexcept {
    n1 = internal::OperatorSome(MathOperator::MATH_SUB, std::move(n1), std::forward<T>(n2));
    return n1;
}

template <typename T1, typename T2>
std::unique_ptr<internal::NodeImpl> operator*(T1 &&n1, T2 &&n2) noexcept {
    return internal::OperatorSome(MathOperator::MATH_MULTIPLY, std::forward<T1>(n1), std::forward<T2>(n2));
}

template <typename T>
std::unique_ptr<internal::NodeImpl> &operator*=(std::unique_ptr<internal::NodeImpl> &n1, T &&n2) noexcept {
    n1 = internal::OperatorSome(MathOperator::MATH_MULTIPLY, std::move(n1), std::forward<T>(n2));
    return n1;
}

template <typename T1, typename T2>
std::unique_ptr<internal::NodeImpl> operator/(T1 &&n1, T2 &&n2) noexcept {
    return internal::OperatorSome(MathOperator::MATH_DIVIDE, std::forward<T1>(n1), std::forward<T2>(n2));
}

template <typename T>
std::unique_ptr<internal::NodeImpl> &operator/=(std::unique_ptr<internal::NodeImpl> &n1, T &&n2) noexcept {
    n1 = internal::OperatorSome(MathOperator::MATH_DIVIDE, std::move(n1), std::forward<T>(n2));
    return n1;
}

//
// class Matrix {
// public:
// private:
//};

} // namespace tomsolver

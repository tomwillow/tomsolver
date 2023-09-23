#pragma once

#include "node.h"

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

namespace internal {

NodeImpl::NodeImpl(const NodeImpl &rhs) noexcept {
    operator=(rhs);
}

NodeImpl &NodeImpl::operator=(const NodeImpl &rhs) noexcept {
    type = rhs.type;
    op = rhs.op;
    value = rhs.value;
    varname = rhs.varname;
    parent = rhs.parent;
    if (rhs.left) {
        left = Clone(rhs.left);
        left->parent = this;
    } else {
        left = nullptr;
    }
    if (rhs.right) {
        right = Clone(rhs.right);
        right->parent = this;
    } else {
        right = nullptr;
    }
    return *this;
}

NodeImpl::NodeImpl(NodeImpl &&rhs) noexcept {
    operator=(std::move(rhs));
}

NodeImpl &NodeImpl::operator=(NodeImpl &&rhs) noexcept {
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

NodeImpl::~NodeImpl() {
    Release();
}

std::string NodeImpl::ToString() const noexcept {
    std::string ret;
    TraverseInOrder(ret);
    return ret;
}

double NodeImpl::Vpa() const {
    return VpaNonRecursively();
}

void NodeImpl::CheckParent() const noexcept {
    // 使用二叉树数的非递归前序遍历。
    std::stack<const NodeImpl *> stk;

    CheckOperatorNum();
    if (right) {
        stk.push(right.get());
    }
    if (left) {
        stk.push(left.get());
    }
    while (!stk.empty()) {
        const NodeImpl *f = stk.top();
        stk.pop();

        // 检查
        assert(f->parent);
        bool isLeftChild = f->parent->left.get() == f;
        bool isRightChild = f->parent->right.get() == f;
        assert(isLeftChild || isRightChild);

        f->CheckOperatorNum();

        if (f->right) {
            stk.push(f->right.get());
        }
        if (f->left) {
            stk.push(f->left.get());
        }
    }
}

void NodeImpl::CheckOperatorNum() const noexcept {
    if (type != NodeType::OPERATOR)
        return;

    if (GetOperatorNum(op) == 1) {
        assert(left);
        assert(right == nullptr);
        return;
    }
    if (GetOperatorNum(op) == 2) {
        assert(left);
        assert(right);
        return;
    }
    assert(0);
}

std::string NodeImpl::NodeToStr() const noexcept {
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

void NodeImpl::TraverseInOrder(std::string &output) const noexcept {
    switch (type) {
    case NodeType::NUMBER:
    case NodeType::VARIABLE:
        output += NodeToStr();
        return;
    }

    int has_parenthesis = 0;
    if (GetOperatorNum(op) == 1) //一元运算符：函数和取负
    {
        // if (type == NodeType::FUNCTION) {
        output += NodeToStr() + "(";
        has_parenthesis = 1;
        //} else {
        //    output += "(" + NodeToStr();
        //    has_parenthesis = 1;
        //}
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

double NodeImpl::VpaRecursively() const {
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

double NodeImpl::VpaNonRecursively() const {
    std::stack<const NodeImpl *> stk;
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
    while (!revertedPostOrder.empty()) {
        auto f = revertedPostOrder.top();
        revertedPostOrder.pop();

        if (f->type == NodeType::NUMBER) {
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

void NodeImpl::Release() noexcept {
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

// TODO: to non-recursively
std::unique_ptr<NodeImpl> Clone(const std::unique_ptr<NodeImpl> &rhs) noexcept {
    auto ret = std::make_unique<NodeImpl>(rhs->type, rhs->op, rhs->value, rhs->varname);
    if (rhs->left) {
        ret->left = Clone(rhs->left);
        ret->left->parent = ret.get();
    }

    if (rhs->right) {
        ret->right = Clone(rhs->right);
        ret->right->parent = ret.get();
    }
    return ret;
}

std::unique_ptr<NodeImpl> Move(std::unique_ptr<NodeImpl> &rhs) noexcept {
    return std::move(rhs);
}

void CopyOrMoveTo(NodeImpl *parent, std::unique_ptr<NodeImpl> &child, std::unique_ptr<NodeImpl> &&n1) noexcept {
    n1->parent = parent;
    child = std::move(n1);
}

void CopyOrMoveTo(NodeImpl *parent, std::unique_ptr<NodeImpl> &child, const std::unique_ptr<NodeImpl> &n1) noexcept {
    auto n1Clone = std::unique_ptr<NodeImpl>(new NodeImpl(*n1));
    n1Clone->parent = parent;
    child = std::move(n1Clone);
}

std::ostream &operator<<(std::ostream &out, const std::unique_ptr<internal::NodeImpl> &n) noexcept {
    out << n->ToString();
    return out;
}

} // namespace internal

std::unique_ptr<internal::NodeImpl> Num(double num) noexcept {
    return std::make_unique<internal::NodeImpl>(NodeType::NUMBER, MathOperator::MATH_NULL, num, "");
}

bool VarNameIsLegal(const std::string &varname) noexcept {
    if (varname.empty()) {
        return false;
    }
    char c = varname[0];
    if (!isalpha(c) && c != '_') {
        return false;
    }
    auto n = varname.size();
    for (size_t i = 1; i < n; ++i) {
        c = varname[i];
        if (isalnum(c) || c == '_') {
            continue;
        }
        return false;
    }
    return true;
}

std::unique_ptr<internal::NodeImpl> Var(const std::string &varname) {
    if (!VarNameIsLegal(varname)) {
        throw std::runtime_error("Illegal varname: " + varname);
    }
    return std::make_unique<internal::NodeImpl>(NodeType::VARIABLE, MathOperator::MATH_NULL, 0, varname);
}

} // namespace tomsolver

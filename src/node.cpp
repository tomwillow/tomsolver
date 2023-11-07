#include "node.h"

#include "config.h"
#include "math_operator.h"

#include <cassert>
#include <forward_list>
#include <iostream>
#include <limits>
#include <memory>
#include <regex>
#include <stack>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace tomsolver {

namespace internal {

NodeImpl::NodeImpl(const NodeImpl &rhs) noexcept {
    *this = rhs;
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
    *this = std::move(rhs);
}

NodeImpl &NodeImpl::operator=(NodeImpl &&rhs) noexcept {
    type = std::exchange(rhs.type, {});
    op = std::exchange(rhs.op, {});
    value = std::exchange(rhs.value, {});
    varname = std::exchange(rhs.varname, {});
    parent = std::exchange(rhs.parent, {});
    left = std::exchange(rhs.left, {});
    if (left) {
        left->parent = this;
    }
    right = std::exchange(rhs.right, {});
    if (right) {
        right->parent = this;
    }

    return *this;
}

NodeImpl::~NodeImpl() {
    Release();
}

// 前序遍历。非递归实现。
bool NodeImpl::Equal(const Node &rhs) const noexcept {
    if (this == rhs.get()) {
        return true;
    }

    std::stack<std::tuple<const NodeImpl &, const NodeImpl &>> stk;

    auto tie = [](const NodeImpl &node) {
        return std::tie(node.type, node.op, node.value, node.varname);
    };

    auto IsSame = [&tie](const NodeImpl &lhs, const NodeImpl &rhs) {
        return tie(lhs) == tie(rhs);
    };

    auto CheckChildren = [&stk](const Node &lhs, const Node &rhs) {
        // ╔═════╦═════╦════════╦═════════╗
        // ║ lhs ║ rhs ║ return ║ emplace ║
        // ╠═════╬═════╬════════╬═════════╣
        // ║ T   ║ T   ║ T      ║ T       ║
        // ╟─────╫─────╫────────╫─────────╢
        // ║ T   ║ F   ║ F      ║ F       ║
        // ╟─────╫─────╫────────╫─────────╢
        // ║ F   ║ T   ║ F      ║ F       ║
        // ╟─────╫─────╫────────╫─────────╢
        // ║ F   ║ F   ║ T      ║ F       ║
        // ╚═════╩═════╩════════╩═════════╝
        if (!lhs ^ !rhs) {
            return false;
        }

        if (lhs && rhs) {
            stk.emplace(*lhs, *rhs);
        }

        return true;
    };

    auto CheckNode = [&IsSame, &CheckChildren](const NodeImpl &lhs, const NodeImpl &rhs) {
        return IsSame(lhs, rhs) && CheckChildren(lhs.left, rhs.left) && CheckChildren(lhs.right, rhs.right);
    };

    if (!CheckNode(*this, *rhs)) {
        return false;
    }

    while (!stk.empty()) {
        const auto &[lhs, rhs] = stk.top();
        stk.pop();

        // 检查
        if (!CheckNode(lhs, rhs)) {
            return false;
        }
    }

    return true;
}

std::string NodeImpl::ToString() const noexcept {
    std::stringstream ss;
    ToStringNonRecursively(ss);
    return ss.str();
}

double NodeImpl::Vpa() const {
    return VpaNonRecursively();
}

NodeImpl &NodeImpl::Calc() {
    auto d = Vpa();
    *this = {};
    value = d;

    return *this;
}

// 前序遍历。非递归实现。
void NodeImpl::CheckParent() const noexcept {
    std::stack<std::tuple<const NodeImpl &>> stk;

    auto EmplaceNode = [&stk](const Node &node) {
        if (node) {
            stk.emplace(*node);
        }
    };
    auto TryEmplaceChildren = [&EmplaceNode](const NodeImpl &node) {
        node.CheckOperatorNum();
        EmplaceNode(node.left);
        EmplaceNode(node.right);
    };

    TryEmplaceChildren(*this);

    while (!stk.empty()) {
        const auto &[f] = stk.top();
        stk.pop();

#ifndef NDEBUG
        // 检查
        assert(f.parent);
        bool isLeftChild = f.parent->left.get() == &f;
        bool isRightChild = f.parent->right.get() == &f;
        assert(isLeftChild || isRightChild);
#endif

        TryEmplaceChildren(f);
    }
}

void NodeImpl::CheckOperatorNum() const noexcept {
    if (type != NodeType::OPERATOR) {
        return;
    }

    switch (GetOperatorNum(op)) {
    case 1:
        assert(!right);
        break;
    case 2:
        assert(right);
        break;
    default:
        assert(0);
        break;
    }

    assert(left);
}

std::string NodeImpl::NodeToStr() const noexcept {
    switch (type) {
    case NodeType::NUMBER:
        return tomsolver::ToString(value);
    case NodeType::VARIABLE:
        return varname;
    case NodeType::OPERATOR:
        return MathOperatorToStr(op);
    }
    assert(0 && "unexpected NodeType. maybe this is a bug.");
    return "";
}

// 中序遍历。递归实现。
void NodeImpl::ToStringRecursively(std::stringstream &output) const noexcept {
    switch (type) {
    case NodeType::NUMBER:
        // 如果当前节点是数值且小于0，且前面是-运算符，那么加括号
        if (value < 0 && parent && parent->right.get() == this && parent->op == MathOperator::MATH_SUB) {
            output << "(" << NodeToStr() << ")";
        } else {
            output << NodeToStr();
        }
        return;
    case NodeType::VARIABLE:
        output << NodeToStr();
        return;
    case NodeType::OPERATOR:
        // pass
        break;
    }

    auto hasParenthesis = false;
    auto operatorNum = GetOperatorNum(op);
    if (operatorNum == 1) // 一元运算符：函数和取负
    {
        if (op == MathOperator::MATH_POSITIVE || op == MathOperator::MATH_NEGATIVE) {
            output << "(" << NodeToStr();
        } else {
            output << NodeToStr() << "(";
        }
        hasParenthesis = true;
    } else {
        // 非一元运算符才输出，即一元运算符的输出顺序已改变
        if (type == NodeType::OPERATOR && parent) { // 本级为运算符
            if ((GetOperatorNum(parent->op) == 2 && // 父运算符存在，为二元，
                 (Rank(parent->op) > Rank(op)       // 父级优先级高于本级->加括号

                  || ( // 两级优先级相等
                         Rank(parent->op) == Rank(op) &&
                         (
                             // 本级为父级的右子树 且父级不满足结合律->加括号
                             (InAssociativeLaws(parent->op) == false && this == parent->right.get()) ||
                             // 两级都是右结合
                             (InAssociativeLaws(parent->op) == false && IsLeft2Right(op) == false)))))

                //||

                ////父运算符存在，为除号，且本级为分子，则添加括号
                //(now->parent->eOperator == MATH_DIVIDE && now == now->parent->right)
            ) {
                output << "(";
                hasParenthesis = true;
            }
        }
    }

    if (left) // 左遍历
    {
        left->ToStringRecursively(output);
    }

    if (operatorNum != 1) // 非一元运算符才输出，即一元运算符的输出顺序已改变
    {
        output << NodeToStr();
    }

    if (right) // 右遍历
    {
        right->ToStringRecursively(output);
    }

    // 回到本级时补齐右括号，包住前面的东西
    if (hasParenthesis) {
        output << ")";
    }
}

// 中序遍历。非递归实现。
void NodeImpl::ToStringNonRecursively(std::stringstream &output) const noexcept {
    std::stack<std::tuple<const NodeImpl &>> stk;

    NodeImpl rightParenthesis(NodeType::OPERATOR, MathOperator::MATH_RIGHT_PARENTHESIS, 0, "");

    auto AddLeftLine = [&stk, &output, &rightParenthesis](const NodeImpl *cur) {
        while (cur) {
            if (cur->type != NodeType::OPERATOR) {
                stk.emplace(*cur);
                cur = cur->left.get();
                continue;
            }

            // 一元运算符的特殊处理：
            //      例如sin: 直接输出 "sin(" ，并且把一个右括号入栈。让退栈时这个右括号能包裹住现在的子树。
            //      如果是+/-: 直接输出 "+"/"-"，如果+/-的操作数是operator，那么处理方式和sin这类一样；
            //                                  如果+/-的操作数是number/variable，那么不加括号。
            if (GetOperatorNum(cur->op) == 1) {
                if ((cur->op == MathOperator::MATH_POSITIVE || cur->op == MathOperator::MATH_NEGATIVE) &&
                    (cur->left->type != NodeType::OPERATOR)) {
                    output << cur->NodeToStr();
                    cur = cur->left.get();
                    continue;
                }
                output << cur->NodeToStr() << "(";

                // not push this op

                // push ')'
                stk.emplace(rightParenthesis);

                cur = cur->left.get();
                continue;
            }

            // 二元运算符的特殊处理：
            if (cur->parent) {
                if ((GetOperatorNum(cur->parent->op) == 2 && // 父运算符存在，为二元，
                     (Rank(cur->parent->op) > Rank(cur->op)  // 父级优先级高于本级->加括号

                      || ( // 两级优先级相等
                             Rank(cur->parent->op) == Rank(cur->op) &&
                             (
                                 // 本级为父级的右子树 且父级不满足结合律->加括号
                                 (InAssociativeLaws(cur->parent->op) == false && cur == cur->parent->right.get()) ||
                                 // 两级都是右结合
                                 (InAssociativeLaws(cur->parent->op) == false && IsLeft2Right(cur->op) == false)))))

                    //||

                    ////父运算符存在，为除号，且本级为分子，则添加括号
                    //(now->parent->eOperator == MATH_DIVIDE && now == now->parent->right)
                ) {
                    output << "(";

                    // push ')'
                    stk.emplace(rightParenthesis);

                    stk.emplace(*cur);
                    cur = cur->left.get();
                    continue;
                }
            }

            stk.emplace(*cur);
            cur = cur->left.get();
        }
    };

    AddLeftLine(this);

    while (!stk.empty()) {
        const auto &[cur] = stk.top();
        stk.pop();

        // output

        // 负数的特殊处理
        // 如果当前节点是数值且小于0，且前面是-运算符，那么加括号
        if (cur.type == NodeType::NUMBER && cur.value < 0 && cur.parent && cur.parent->right.get() == &cur &&
            cur.parent->op == MathOperator::MATH_SUB) {
            output << "(" << cur.NodeToStr() << ")";
        } else {
            output << cur.NodeToStr();
        }

        if (cur.right) {
            AddLeftLine(cur.right.get());
            continue;
        }
    }
}

// 后序遍历。递归实现。
double NodeImpl::VpaRecursively() const {

    auto vpa = [](const Node &node) {
        return node ? node->Vpa() : 0;
    };

    switch (type) {
    case NodeType::NUMBER:
        return value;

    case NodeType::VARIABLE:
        throw std::runtime_error("has variable. can not calculate to be a number");

    case NodeType::OPERATOR:
        assert((GetOperatorNum(op) == 1 && left && right == nullptr) || (GetOperatorNum(op) == 2 && left && right));
        return tomsolver::Calc(op, vpa(left), vpa(right));
    }

    throw std::runtime_error("unsupported node type");
}

// 后序遍历。非递归实现。
double NodeImpl::VpaNonRecursively() const {

    std::stack<std::tuple<const NodeImpl &>> stk;
    std::forward_list<std::tuple<const NodeImpl &>> revertedPostOrder;

    // ==== Part I ====

    // 借助一个栈，得到反向的后序遍历序列，结果保存在revertedPostOrder
    stk.emplace(*this);

    while (!stk.empty()) {
        const auto &[node] = stk.top();
        stk.pop();

        if (node.left) {
            stk.emplace(*node.left);
        }

        if (node.right) {
            stk.emplace(*node.right);
        }

        revertedPostOrder.emplace_front(node);
    }

    // ==== Part II ====
    // revertedPostOrder的反向序列是一组逆波兰表达式，根据这组逆波兰表达式可以计算出表达式的值
    // calcStk是用来计算值的临时栈，计算完成后calcStk的size应该为1
    std::stack<double> calcStk;
    // for (auto it = revertedPostOrder.rbegin(); it != revertedPostOrder.rend(); ++it) {
    for (const auto &[node] : revertedPostOrder) {
        switch (node.type) {
        case NodeType::NUMBER:
            calcStk.emplace(node.value);
            break;

        case NodeType::OPERATOR: {
            auto r = std::numeric_limits<double>::quiet_NaN();

            switch (GetOperatorNum(node.op)) {
            case 1:
                break;
            case 2:
                r = calcStk.top();
                calcStk.pop();
                break;
            default:
                assert(0 && "[VpaNonRecursively] unsupported operator num");
                break;
            }

            auto &l = calcStk.top();
            l = tomsolver::Calc(node.op, l, r);
            break;
        }

        default:
            throw std::runtime_error("wrong");
            break;
        }
    }

    assert(calcStk.size() == 1);

    return calcStk.top();
}

// 后序遍历。因为要在左右儿子都没有的情况下删除节点。
void NodeImpl::Release() noexcept {
    std::stack<Node> stk;

    auto emplaceNode = [&stk](Node node) {
        if (node) {
            stk.emplace(std::move(node));
        }
    };

    auto emplaceChildren = [&emplaceNode](NodeImpl &node) {
        emplaceNode(std::move(node.left));
        emplaceNode(std::move(node.right));
    };

    emplaceChildren(*this);

    while (!stk.empty()) {
        auto node = std::move(stk.top());
        stk.pop();

        emplaceChildren(*node);

        assert(!node->left && !node->right);

        // 这里如果把node填入vector，最后翻转。得到的序列就是后序遍历。

        // 这里node会被自动释放。
    }
}

Node CloneRecursively(const Node &src) noexcept {
    auto ret = std::make_unique<NodeImpl>(src->type, src->op, src->value, src->varname);
    auto Copy = [ret = ret.get()](Node &tgt, const Node &src) {
        if (src) {
            tgt = Clone(src);
            tgt->parent = ret;
        }
    };

    Copy(ret->left, src->left);
    Copy(ret->right, src->right);

    return ret;
}

// 前序遍历。非递归实现。
Node CloneNonRecursively(const Node &src) noexcept {
    std::stack<std::tuple<const NodeImpl &, NodeImpl &, Node &>> stk;

    auto MakeNode = [](const NodeImpl &src, NodeImpl *parent = nullptr) {
        auto node = std::make_unique<NodeImpl>(src.type, src.op, src.value, src.varname);
        node->parent = parent;
        return node;
    };

    auto EmplaceNode = [&stk](const Node &src, NodeImpl &parent, Node &tgt) {
        if (src) {
            stk.emplace(*src, parent, tgt);
        }
    };

    auto EmplaceChildren = [&EmplaceNode](const NodeImpl &src, Node &tgt) {
        EmplaceNode(src.left, *tgt, tgt->left);
        EmplaceNode(src.right, *tgt, tgt->right);
    };

    auto ret = MakeNode(*src);
    EmplaceChildren(*src, ret);

    while (!stk.empty()) {
        const auto &[src, parent, tgt] = stk.top();
        stk.pop();

        tgt = MakeNode(src, &parent);
        EmplaceChildren(src, tgt);
    }

    return ret;
}

void CopyOrMoveTo(NodeImpl *parent, Node &child, Node &&n1) noexcept {
    n1->parent = parent;
    child = std::move(n1);
}

void CopyOrMoveTo(NodeImpl *parent, Node &child, const Node &n1) noexcept {
    auto n1Clone = std::make_unique<NodeImpl>(*n1);
    n1Clone->parent = parent;
    child = std::move(n1Clone);
}

std::ostream &operator<<(std::ostream &out, const Node &n) noexcept {
    out << n->ToString();
    return out;
}

Node Operator(MathOperator op, Node left, Node right) noexcept {
    auto ret = std::make_unique<internal::NodeImpl>(NodeType::OPERATOR, op, 0, "");

    auto SetChild = [ret = ret.get()](Node &tgt, Node src) {
        if (src) {
            src->parent = ret;
            tgt = std::move(src);
        }
    };

    SetChild(ret->left, std::move(left));
    SetChild(ret->right, std::move(right));

    return ret;
}

// 前序遍历。非递归实现。
std::set<std::string> NodeImpl::GetAllVarNames() const noexcept {
    std::set<std::string> ret;

    std::stack<std::tuple<const NodeImpl &>> stk;

    auto EmplaceNode = [&stk](const Node &node) {
        if (node) {
            stk.emplace(*node);
        }
    };

    auto EmplaceChild = [&ret, &EmplaceNode](const NodeImpl &node) {
        if (node.type == NodeType::VARIABLE) {
            ret.emplace(node.varname);
        }
        EmplaceNode(node.left);
        EmplaceNode(node.right);
    };

    EmplaceChild(*this);

    while (!stk.empty()) {
        const auto &[node] = stk.top();
        stk.pop();
        EmplaceChild(node);
    }

    return ret;
}

} // namespace internal

Node Clone(const Node &rhs) noexcept {
    return internal::CloneNonRecursively(rhs);
}

Node Move(Node &rhs) noexcept {
    return std::move(rhs);
}

Node Num(double num) noexcept {
    return std::make_unique<internal::NodeImpl>(NodeType::NUMBER, MathOperator::MATH_NULL, num, "");
}

Node Op(MathOperator op) {
    if (op == MathOperator::MATH_NULL) {
        throw std::runtime_error("Illegal MathOperator: MATH_NULL");
    }
    return std::make_unique<internal::NodeImpl>(NodeType::OPERATOR, op, 0, "");
}

bool VarNameIsLegal(std::string_view varname) noexcept {
    return std::regex_match(varname.begin(), varname.end(), std::regex{R"((?=\w)\D\w*)"});
}

Node Var(std::string_view varname) {
    auto name = std::string{varname};
    if (!VarNameIsLegal(varname)) {
        throw std::runtime_error("Illegal varname: " + name);
    }
    return std::make_unique<internal::NodeImpl>(NodeType::VARIABLE, MathOperator::MATH_NULL, 0, std::move(name));
}

} // namespace tomsolver

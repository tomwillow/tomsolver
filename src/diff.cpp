#include "diff.h"

#include "functions.h"
#include "simplify.h"

#include <queue>
#include <cmath> // std::log

namespace tomsolver {

namespace internal {

class DiffFunctions {
public:
    struct DiffNode {
        NodeImpl &node;
        const bool isLeftChild;

        DiffNode(NodeImpl &node) : node(node), isLeftChild(node.parent && node.parent->left.get() == &node) {}
    };

    static void DiffOnce(Node &root, const std::string &varname) {
        std::queue<DiffNode> q;

        if (root->type == NodeType::OPERATOR) {
            DiffOnceOperator(root, q);
        } else {
            q.emplace(*root);
        }

        while (!q.empty()) {
            auto& node = q.front().node;
            auto isLeftChild = q.front().isLeftChild;
            q.pop();

            switch (node.type) {
            case NodeType::VARIABLE:
                node.type = NodeType::NUMBER;
                node.value = node.varname == varname ? 1 : 0;
                node.varname = "";
                break;

            case NodeType::NUMBER:
                node.value = 0;
                break;

            case NodeType::OPERATOR: {
                auto &child = isLeftChild ? node.parent->left : node.parent->right;
                DiffOnceOperator(child, q);
                break;
            }
            default:
                assert(0 && "inner bug");
            }
        }
    }

    static void DiffOnceOperator(Node &node, std::queue<DiffNode> &q) {
        auto parent = node->parent;

        // 调用前提：node是1元操作符
        // 如果node的成员是数字，那么整个node变为数字节点，value=0，且返回true
        // 例如： sin(1)' = 0
        auto CullNumberMember = [&node]() -> bool {
            assert(GetOperatorNum(node->op) == 1);
            assert(node->left);
            if (node->left->type == NodeType::NUMBER) {
                node->left = nullptr;
                node->type = NodeType::NUMBER;
                node->op = MathOperator::MATH_NULL;
                node->value = 0.0;
                return true;
            }
            return false;
        };

        // 调用前提：node是2元操作符
        // 如果node的成员是数字，那么整个node变为数字节点，value=0，且返回true
        // 例如： (2*3)' = 0
        auto CullNumberMemberBinary = [&node]() -> bool {
            assert(GetOperatorNum(node->op) == 2);
            assert(node->left && node->right);
            if (node->left->type == NodeType::NUMBER && node->right->type == NodeType::NUMBER) {
                node->left = nullptr;
                node->right = nullptr;
                node->type = NodeType::NUMBER;
                node->op = MathOperator::MATH_NULL;
                node->value = 0.0;
                return true;
            }
            return false;
        };

        switch (node->op) {
        case MathOperator::MATH_NULL: {
            assert(0 && "inner bug");
            break;
        }
        case MathOperator::MATH_POSITIVE:
        case MathOperator::MATH_NEGATIVE: {
            q.emplace(*node->left);
            return;
        }

        // 函数
        case MathOperator::MATH_SIN: {
            if (CullNumberMember()) {
                return;
            }

            // sin(u)' = cos(u) * u'
            node->op = MathOperator::MATH_COS;
            auto u2 = Clone(node->left);
            q.emplace(*u2);
            node = Move(node) * Move(u2);
            node->parent = parent;
            break;
        }
        case MathOperator::MATH_COS: {
            if (CullNumberMember()) {
                return;
            }

            // cos(u)' = -sin(u) * u'
            node->op = MathOperator::MATH_SIN;
            auto u2 = Clone(node->left);
            q.emplace(*u2);
            node = -Move(node) * Move(u2);
            node->parent = parent;
            break;
        }
        case MathOperator::MATH_TAN: {
            if (CullNumberMember()) {
                return;
            }

            // tan'u = 1/(cos(u)^2) * u'
            node->op = MathOperator::MATH_COS;
            auto &u = node->left;
            auto u2 = Clone(u);
            q.emplace(*u2);
            node = Num(1) / (Move(node) ^ Num(2)) * Move(u2);
            node->parent = parent;
            return;
        }
        case MathOperator::MATH_ARCSIN: {
            if (CullNumberMember()) {
                return;
            }

            // asin'u = 1/sqrt(1-u^2) * u'
            auto &u = node->left;
            auto u2 = Clone(u);
            q.emplace(*u2);
            node = (Num(1) / sqrt(Num(1) - (Move(u) ^ Num(2)))) * Move(u2);
            node->parent = parent;
            return;
        }
        case MathOperator::MATH_ARCCOS: {
            if (CullNumberMember()) {
                return;
            }

            // acos'u = -1/sqrt(1-u^2) * u'
            auto &u = node->left;
            auto u2 = Clone(u);
            q.emplace(*u2);
            node = (Num(-1) / sqrt(Num(1) - (Move(u) ^ Num(2)))) * Move(u2);
            node->parent = parent;
            return;
        }
        case MathOperator::MATH_ARCTAN: {
            if (CullNumberMember()) {
                return;
            }

            // atan'u = 1/(1+u^2) * u'
            auto &u = node->left;
            auto u2 = Clone(u);
            q.emplace(*u2);
            node = (Num(1) / (Num(1) + (Move(u) ^ Num(2)))) * Move(u2);
            node->parent = parent;
            return;
        }
        case MathOperator::MATH_SQRT: {
            if (CullNumberMember()) {
                return;
            }

            // sqrt(u)' = 1/(2*sqrt(u)) * u'
            auto &u = node->left;
            auto u2 = Clone(u);
            q.emplace(*u2);
            node = Num(1) / (Num(2) * Move(node)) * Move(u2);
            node->parent = parent;
            return;
        }
        case MathOperator::MATH_LOG: {
            if (CullNumberMember()) {
                return;
            }

            // ln(u)' = 1/u * u'
            auto &u = node->left;
            auto u2 = Clone(u);
            q.emplace(*u2);
            node = (Num(1) / Move(u)) * Move(u2);
            node->parent = parent;
            return;
        }
        case MathOperator::MATH_LOG2: {
            if (CullNumberMember()) {
                return;
            }

            // loga(u)' = 1/(u * ln(a)) * u'
            auto a = 2.0;
            auto &u = node->left;
            auto u2 = Clone(u);
            q.emplace(*u2);
            node = (Num(1) / (Move(u) * Num(std::log(a)))) * Move(u2);
            node->parent = parent;
            return;
        }
        case MathOperator::MATH_LOG10: {
            if (CullNumberMember()) {
                return;
            }

            // loga(u)' = 1/(u * ln(a)) * u'
            auto a = 10.0;
            auto &u = node->left;
            auto u2 = Clone(u);
            q.emplace(*u2);
            node = (Num(1) / (Move(u) * Num(std::log(a)))) * Move(u2);
            node->parent = parent;
            return;
        }
        case MathOperator::MATH_EXP: {
            if (CullNumberMember()) {
                return;
            }

            // e^x=e^x
            if (node->left->type == NodeType::VARIABLE)
                return;

            // (e^u)' = e^u * u'
            auto u2 = Clone(node->left);
            q.emplace(*u2);
            node = Move(node) * Move(u2);
            node->parent = parent;
            break;
        }

        // 二元
        case MathOperator::MATH_ADD:
        case MathOperator::MATH_SUB:
            if (CullNumberMemberBinary()) {
                return;
            }
            // (u + v)' = u' + v'
            if (node->left) {
                q.emplace(*node->left);
            }
            if (node->right) {
                q.emplace(*node->right);
            }
            return;
        case MathOperator::MATH_MULTIPLY: {
            // 两个操作数中有一个是数字
            if ( node->left->type == NodeType::NUMBER) {
                q.emplace(*node->right);
                return;
            }
            if (node->right->type == NodeType::NUMBER) {
                q.emplace(*node->left);
                return;
            }

            if (CullNumberMemberBinary()) {
                return;
            }

            // (u*v)' = u' * v + u * v'
            auto &u = node->left;
            auto &v = node->right;
            q.emplace(*u);
            auto u2 = Clone(u);
            auto v2 = Clone(v);
            q.emplace(*v2);
            node = Move(node) + Move(u2) * Move(v2);
            node->parent = parent;
            return;
        }
        case MathOperator::MATH_DIVIDE: {
            // auto leftIsNumber = node->left->type == NodeType::NUMBER;
            auto rightIsNumber = node->right->type == NodeType::NUMBER;

            // f(x)/number = f'(x)/number
            if (rightIsNumber) {
                q.emplace(*node->left);
                return;
            }

            if (CullNumberMemberBinary()) {
                return;
            }

            // (u/v)' = (u'v - uv')/(v^2)
            auto &u = node->left;
            auto &v = node->right;
            auto u2 = Clone(u);
            auto v2 = Clone(v);
            auto v3 = Clone(v);
            q.emplace(*u);
            q.emplace(*v2);
            node = (Move(u) * Move(v) - Move(u2) * Move(v2)) / (Move(v3) ^ Num(2));
            node->parent = parent;
            return;
        }
        case MathOperator::MATH_POWER: {
            // 如果两个操作数都是数字
            if (CullNumberMemberBinary()) {
                return;
            }

            auto lChildIsNumber = node->left->type == NodeType::NUMBER;
            auto rChildIsNumber = node->right->type == NodeType::NUMBER;

            // (u^a)' = a*u^(a-1) * u'
            if (rChildIsNumber) {
                auto &a = node->right;
                auto aValue = a->value;
                auto &u = node->left;
                auto u2 = Clone(u);
                q.emplace(*u2);
                node = std::move(a) * (std::move(u) ^ Num(aValue - 1)) * std::move(u2);
                node->parent = parent;
                return;
            }

            // (a^x)' = a^x * ln(a)  when a>0 and a!=1
            if (lChildIsNumber) {
                auto &a = node->left;
                auto aValue = a->value;
                auto &u = node->right;
                auto u2 = Clone(u);
                q.emplace(*u2);
                node = (std::move(a) ^ std::move(u)) * log(Num(aValue)) * std::move(u2);
                node->parent = parent;
                return;
            }

            // (u^v)' = ( e^(v*ln(u)) )' = e^(v*ln(u)) * (v*ln(u))' = u^v * (v*ln(u))'
            // 左右都不是数字
            auto &u = node->left;
            auto &v = node->right;
            auto vln_u = Clone(v) * log(Clone(u));
            q.emplace(*vln_u);
            node = Move(node) * std::move(vln_u);
            node->parent = parent;
            return;
        }

        case MathOperator::MATH_AND: {
            throw std::runtime_error("can not apply diff for AND(&) operator");
            return;
        }
        case MathOperator::MATH_OR: {
            throw std::runtime_error("can not apply diff for OR(|) operator");
            return;
        }
        case MathOperator::MATH_MOD: {
            throw std::runtime_error("can not apply diff for MOD(%) operator");
            return;
        }
        case MathOperator::MATH_LEFT_PARENTHESIS:
        case MathOperator::MATH_RIGHT_PARENTHESIS:
            assert(0 && "inner bug");
            return;
        default:
            assert(0 && "inner bug");
            return;
        }
    }
};

} // namespace internal

Node Diff(const Node &node, const std::string &varname, int i) {
    auto node2 = Clone(node);
    return Diff(std::move(node2), varname, i);
}

Node Diff(Node &&node, const std::string &varname, int i) {
    assert(i > 0);
    auto n = std::move(node);
    while (i--) {
        internal::DiffFunctions::DiffOnce(n, varname);
    }
#ifndef NDEBUG
    auto s = n->ToString();
    n->CheckParent();
#endif
    Simplify(n);
#ifndef NDEBUG
    n->CheckParent();
#endif
    return n;
}

} // namespace tomsolver

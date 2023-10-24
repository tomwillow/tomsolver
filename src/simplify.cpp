#include "simplify.h"

#include <iostream>
#include <stack>
#include <algorithm>
#include <cassert>

namespace tomsolver {

namespace internal {

class SimplifyFunctions {
public:
    struct SimplifyNode {
        NodeImpl *node;
        bool isLeftChild;

        SimplifyNode(NodeImpl *node, bool isLeftChild) : node(node), isLeftChild(isLeftChild) {
            if (node->parent) {
                if (node->parent->left.get() == node) {
                    if (!isLeftChild) {
                        assert(0 && "inner bug");
                    }
                } else {
                    if (isLeftChild) {
                        assert(0 && "inner bug");
                    }
                }
            }
        }
    };

    static void SimplifySingleNode(std::unique_ptr<NodeImpl> &n) noexcept {
        const NodeImpl *parent = n->parent;
        // 对单节点n进行化简。

        // 对于1元运算符，且儿子是数字的，直接计算出来
        if (GetOperatorNum(n->op) == 1 && n->left->type == NodeType::NUMBER) {
            n->type = NodeType::NUMBER;
            n->value = tomsolver::Calc(n->op, n->left->value, 0);
            n->op = MathOperator::MATH_NULL;
            n->left = nullptr;
            return;
        }

        // 对于2元运算符
        if (GetOperatorNum(n->op) == 2) {
            // 儿子是数字的，直接计算出来
            if (n->left->type == NodeType::NUMBER && n->right->type == NodeType::NUMBER) {
                n->type = NodeType::NUMBER;
                n->value = tomsolver::Calc(n->op, n->left->value, n->right->value);
                n->op = MathOperator::MATH_NULL;
                n->left = nullptr;
                n->right = nullptr;
                return;
            }

            assert(n->left && n->right);
            bool lChildIs0 = n->left->type == NodeType::NUMBER && n->left->value == 0.0;
            bool rChildIs0 = n->right->type == NodeType::NUMBER && n->right->value == 0.0;
            bool lChildIs1 = n->left->type == NodeType::NUMBER && n->left->value == 1.0;
            bool rChildIs1 = n->right->type == NodeType::NUMBER && n->right->value == 1.0;

            //任何数乘或被乘0、被0除、0的除0外的任何次方，等于0
            if ((n->op == MathOperator::MATH_MULTIPLY && (lChildIs0 || rChildIs0)) ||
                (n->op == MathOperator::MATH_DIVIDE && lChildIs0) || (n->op == MathOperator::MATH_POWER && lChildIs0)) {
                n = Num(0);
                n->parent = parent;
                return;
            }

            //任何数加或被加0、被减0、乘或被乘1、被1除、开1次方，等于自身
            if ((n->op == MathOperator::MATH_ADD && (lChildIs0 || rChildIs0)) ||
                (n->op == MathOperator::MATH_SUB && rChildIs0) ||
                (n->op == MathOperator::MATH_MULTIPLY && (lChildIs1 || rChildIs1)) ||
                (n->op == MathOperator::MATH_DIVIDE && rChildIs1) || (n->op == MathOperator::MATH_POWER && rChildIs1)) {

                if (lChildIs1 || lChildIs0) {
                    n->left = nullptr;
                    n = Move(n->right);
                    n->parent = parent;
                } else if (rChildIs1 || rChildIs0) {
                    n->right = nullptr;
                    n = Move(n->left);
                    n->parent = parent;
                }
                return;
            }
        }
    }

    static void SimplifyWholeNode(Node &node) {
        using SimplifyNode = internal::SimplifyFunctions::SimplifyNode;
        // 后序遍历。非递归实现。

        // 借助一个栈，得到反向的后序遍历序列，结果保存在revertedPostOrder。除了root节点，root节点不保存在revertedPostOrder里，最后单独化简。
        std::stack<SimplifyNode> stk;
        std::deque<SimplifyNode> revertedPostOrder;

        // ==== Part I ====

        if (node->type != NodeType::OPERATOR) {
            return;
        }

        stk.push(SimplifyNode(node.get(), true));

        while (1) {
            if (stk.empty()) {
                break;
            }

            auto f = stk.top();
            auto n = f.node;
            stk.pop();

            if (n->left && n->left->type == NodeType::OPERATOR) {
                stk.push(SimplifyNode(n->left.get(), true));
            }

            if (n->right && n->right->type == NodeType::OPERATOR) {
                stk.push(SimplifyNode(n->right.get(), false));
            }

            revertedPostOrder.push_back(f);
        }

        // pop掉root，root最后单独处理
        revertedPostOrder.pop_front();

        // ==== Part II ====
        std::for_each(revertedPostOrder.rbegin(), revertedPostOrder.rend(), [](SimplifyNode &snode) {
            if (snode.isLeftChild) {
                Node &n = const_cast<NodeImpl *>(snode.node->parent)->left;
                SimplifyFunctions::SimplifySingleNode(n);
            } else {
                Node &n = const_cast<NodeImpl *>(snode.node->parent)->right;
                SimplifyFunctions::SimplifySingleNode(n);
            }
        });

        SimplifyFunctions::SimplifySingleNode(node);

        return;

        // if (GetOperateNum(now->eOperator) == 2) {

        //    //任何节点的0次方均等于1，除了0的0次方已在前面报错
        //    if (now->eOperator == MATH_POWER && RChildIs0) {
        //        //替换掉当前运算符，这个1节点将在回溯时处理
        //        //新建一个1节点
        //        TNode *temp;
        //        temp = new TNode;
        //        temp->eType = NODE_NUMBER;
        //        temp->value = 1;

        //        // 0节点连接到上面
        //        if (now != head) {
        //            if (now->parent->left == now) {
        //                now->parent->left = temp;
        //                temp->parent = now->parent;
        //            }
        //            if (now->parent->right == now) {
        //                now->parent->right = temp;
        //                temp->parent = now->parent;
        //            }
        //        } else
        //            head = temp;

        //        DeleteNode(now);
        //    }

        //    // 0-x=-x
        //    if (now->eOperator == MATH_SUB && LChildIs0) {
        //        TNode *LChild = now->left;
        //        TNode *RChild = now->right;
        //        now->eOperator = MATH_NEGATIVE;
        //        now->left = RChild;
        //        now->right = NULL;

        //        delete LChild;
        //    }

        //}
    }
};

} // namespace internal

void Simplify(Node &node) noexcept {
    internal::SimplifyFunctions::SimplifyWholeNode(node);
    return;
}

} // namespace tomsolver

#include "simplify.h"

#include <iostream>
#include <stack>
#include <algorithm>
#include <cassert>

namespace tomsolver {

namespace internal {

NodeImpl &NodeImpl::Simplify() noexcept {
    // 后序遍历。非递归实现。

    std::stack<NodeImpl *> stk;
    std::deque<NodeImpl *> revertedPostOrder;

    // ==== Part I ====

    // 借助一个栈，得到反向的后序遍历序列，结果保存在revertedPostOrder
    if (type == NodeType::OPERATOR) {
        stk.push(this);
    }

    while (1) {
        if (stk.empty()) {
            break;
        }

        auto f = stk.top();
        stk.pop();

        if (f->left && f->left->type == NodeType::OPERATOR) {
            stk.push(f->left.get());
        }

        if (f->right && f->right->type == NodeType::OPERATOR) {
            stk.push(f->right.get());
        }

        revertedPostOrder.push_back(f);
    }

    // ==== Part II ====
    std::for_each(revertedPostOrder.rbegin(), revertedPostOrder.rend(), [](NodeImpl *n) {
        // 计算出来
        if (GetOperatorNum(n->op) == 1 && n->left->type == NodeType::NUMBER) {
            n->type = NodeType::NUMBER;
            n->value = Calc(n->op, n->left->value, 0);
            n->op = MathOperator::MATH_NULL;
            n->left = nullptr;
            return;
        }

        if (GetOperatorNum(n->op) == 2 && n->left->type == NodeType::NUMBER && n->right->type == NodeType::NUMBER) {
            n->type = NodeType::NUMBER;
            n->value = Calc(n->op, n->left->value, n->right->value);
            n->op = MathOperator::MATH_NULL;
            n->left = nullptr;
            n->right = nullptr;
            return;
        }
    });

    return *this;

    // if (GetOperateNum(now->eOperator) == 2) {

    //    //若左右儿子都是数字，则计算出来
    //    if (now->left->eType == NODE_NUMBER && now->right->eType == NODE_NUMBER) {
    //        // try
    //        //{
    //        CalcNode(now, now->left, now->right);
    //        delete now->left;
    //        delete now->right;
    //        now->eOperator = MATH_NULL;
    //        now->left = NULL;
    //        now->right = NULL;
    //        //}
    //        // catch (enumError err)
    //        //{
    //        //	return nowError = err;
    //        //}
    //    }

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

    //    //任何数乘或被乘0、被0除、0的除0外的任何次方，等于0
    //    if ((now->eOperator == MATH_MULTIPLY && (LChildIs0 || RChildIs0)) ||
    //        (now->eOperator == MATH_DIVIDE && LChildIs0) || (now->eOperator == MATH_POWER && LChildIs0)) {
    //        //替换掉当前运算符，这个0节点将在回溯时处理
    //        //新建一个0节点
    //        TNode *temp;
    //        temp = new TNode;
    //        temp->eType = NODE_NUMBER;
    //        temp->value = 0;

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

    //    //任何数加或被加0、被减0、乘或被乘1、被1除、开1次方，等于自身
    //    if ((now->eOperator == MATH_ADD && (LChildIs0 || RChildIs0)) || (now->eOperator == MATH_SUB && RChildIs0) ||
    //        (now->eOperator == MATH_MULTIPLY && (LChildIs1 || RChildIs1)) ||

    //        (now->eOperator == MATH_DIVIDE && RChildIs1) || (now->eOperator == MATH_POWER && RChildIs1)) {
    //        TNode *remain = NULL, *num = NULL;
    //        if (LChildIs1 || LChildIs0) {
    //            num = now->left;
    //            remain = now->right;
    //        }
    //        if (RChildIs1 || RChildIs0) {
    //            num = now->right;
    //            remain = now->left;
    //        }

    //        //连接父级和剩余项
    //        if (now != head) {
    //            if (now->parent->left == now) {
    //                now->parent->left = remain;
    //                remain->parent = now->parent;
    //            }
    //            if (now->parent->right == now) {
    //                now->parent->right = remain;
    //                remain->parent = now->parent;
    //            }
    //        } else {
    //            head = remain;
    //            head->parent = NULL;
    //        }
    //        delete num;
    //        delete now;
    //    }
    //}
}

} // namespace internal

Node Simplify(const Node &node) noexcept {
    Node cloned = Clone(node);
    return Simplify(std::move(cloned));
}

Node Simplify(Node &&node) noexcept {
    Node n = std::move(node);
    n->Simplify();
    return n;
}

} // namespace tomsolver

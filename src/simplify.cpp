#include "node.h"

#include "config.h"
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

// TODO 非递归
void NodeImpl::Simplify() noexcept {
    if (type != NodeType::OPERATOR) {
        return;
    }

    // 左遍历
    if (left != nullptr)
        left->Simplify();

    // 右遍历
    if (right != nullptr)
        right->Simplify();

    // 计算出来
    if (GetOperatorNum(op) == 1 && left->type == NodeType::NUMBER) {
        type = NodeType::NUMBER;
        value = Calc(op, left->value, 0);
        op = MathOperator::MATH_NULL;
        left = nullptr;
        return;
    }

    if (GetOperatorNum(op) == 2 && left->type == NodeType::NUMBER && right->type == NodeType::NUMBER) {
        type = NodeType::NUMBER;
        value = Calc(op, left->value, right->value);
        op = MathOperator::MATH_NULL;
        left = nullptr;
        right = nullptr;
        return;
    }

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

} // namespace tomsolver

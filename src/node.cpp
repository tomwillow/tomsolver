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

/**
 * 对于一个节点n和另一个节点n1，把n1移动到作为n的子节点。
 */
void CopyOrMoveTo(NodeImpl *parent, std::unique_ptr<NodeImpl> &child, std::unique_ptr<NodeImpl> &&n1) noexcept {
    n1->parent = parent;
    child = std::move(n1);
}

/**
 * 对于一个节点n和另一个节点n1，把n1整个拷贝一份，把拷贝的副本设为n的子节点。
 */
void CopyOrMoveTo(NodeImpl *parent, std::unique_ptr<NodeImpl> &child, const std::unique_ptr<NodeImpl> &n1) noexcept {
    auto n1Clone = std::unique_ptr<NodeImpl>(new NodeImpl(*n1));
    n1Clone->parent = parent;
    child = std::move(n1Clone);
}

/**
 * 重载std::ostream的<<操作符以输出一个Node节点。
 */
std::ostream &operator<<(std::ostream &out, const std::unique_ptr<internal::NodeImpl> &n) noexcept {
    out << n->ToString();
    return out;
}

} // namespace internal


/**
 * 新建一个数值节点。
 */
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

/**
 * 新建一个变量节点。
 * @exception runtime_error 名字不合法
 */
std::unique_ptr<internal::NodeImpl> Var(const std::string &varname) {
    if (!VarNameIsLegal(varname)) {
        throw std::runtime_error("Illegal varname: " + varname);
    }
    return std::make_unique<internal::NodeImpl>(NodeType::VARIABLE, MathOperator::MATH_NULL, 0, varname);
}

} // namespace tomsolver

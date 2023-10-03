#pragma once

#include "node.h"
#include "matrix.h"

namespace tomsolver {

/**
 * 用node节点替换var指定的变量。
 */
Node Subs(const Node &node, const std::string &oldVar, const Node &newNode) noexcept;

/**
 * 用node节点替换var指定的变量。
 */
Node Subs(Node &&node, const std::string &oldVar, const Node &newNode) noexcept;

/**
 * 用node节点替换var指定的变量。
 */
Node Subs(const Node &node, const std::vector<std::string> &oldVars, const Vec &newNodes) noexcept;

/**
 * 用node节点替换var指定的变量。
 */
Node Subs(Node &&node, const std::vector<std::string> &oldVars, const Vec &newNodes) noexcept;

} // namespace tomsolver

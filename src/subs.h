#pragma once

#include "node.h"
#include "matrix.h"

namespace tomsolver {

/**
 * 用newNode节点替换oldVar指定的变量。
 */
Node Subs(const Node &node, const std::string &oldVar, const Node &newNode) noexcept;

/**
 * 用newNode节点替换oldVar指定的变量。
 */
Node Subs(Node &&node, const std::string &oldVar, const Node &newNode) noexcept;

/**
 * 用newNodes节点替换oldVars指定的变量。
 */
Node Subs(const Node &node, const std::vector<std::string> &oldVars, const Vec &newNodes) noexcept;

/**
 * 用newNodes节点替换oldVars指定的变量。
 */
Node Subs(Node &&node, const std::vector<std::string> &oldVars, const Vec &newNodes) noexcept;

/**
 * 用newNodes节点替换oldVars指定的变量。
 */
Node Subs(const Node &node, const std::unordered_map<std::string, Node> &dict) noexcept;

/**
 * 用newNodes节点替换oldVars指定的变量。
 */
Node Subs(Node &&node, const std::unordered_map<std::string, Node> &dict) noexcept;

/**
 * 用newNodes节点替换oldVars指定的变量。
 */
Node Subs(const Node &node, const std::unordered_map<std::string, double> &varValues) noexcept;

/**
 * 用newNodes节点替换oldVars指定的变量。
 */
Node Subs(Node &&node, const std::unordered_map<std::string, double> &varValues) noexcept;

} // namespace tomsolver

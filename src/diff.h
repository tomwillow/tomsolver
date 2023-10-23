#pragma once

#include "node.h"

namespace tomsolver {

/**
 * node对varname求导。在node包含多个变量时，是对varname求偏导。
 * @exception runtime_error 如果表达式内包含AND(&) OR(|) MOD(%)这类不能求导的运算符，则抛出异常
 */
Node Diff(const Node &node, const std::string &varname, int i = 1);

/**
 * node对varname求导。在node包含多个变量时，是对varname求偏导。
 * @exception runtime_error 如果表达式内包含AND(&) OR(|) MOD(%)这类不能求导的运算符，则抛出异常
 */
Node Diff(Node &&node, const std::string &varname, int i = 1);

} // namespace tomsolver

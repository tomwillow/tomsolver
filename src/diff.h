#pragma once

#include "node.h"

namespace tomsolver {

Node Diff(const Node &node, const std::string &varname, int i = 1) noexcept;

Node Diff(Node &&node, const std::string &varname, int i = 1) noexcept;

} // namespace tomsolver

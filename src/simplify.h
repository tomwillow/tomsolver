#pragma once
#include "node.h"

namespace tomsolver {

Node Simplify(const Node &node) noexcept;

Node Simplify(Node &&node) noexcept;

} // namespace tomsolver

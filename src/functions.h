#pragma once
#include "node.h"

namespace tomsolver {

template <typename T>
Node sin(T &&n) noexcept {
    auto ret = std::make_unique<NodeImpl>(NodeType::OPERATOR, MathOperator::MATH_SIN, 0, "");
    internal::CopyOrMoveTo(ret.get(), ret->left, std::forward<T>(n));
    return ret;
}

} // namespace tomsolver
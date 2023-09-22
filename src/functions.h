#pragma once
#include "node.h"

namespace tomsolver {

template <typename T>
Node sin(T &&n) noexcept {
    return internal::UnaryOperator(MathOperator::MATH_SIN, std::forward<T>(n));
}

} // namespace tomsolver
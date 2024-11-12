#pragma once
#include "node.h"

namespace tomsolver {

template <typename T>
Node sin(T &&n) noexcept {
    return internal::UnaryOperator(MathOperator::MATH_SIN, std::forward<T>(n));
}

template <typename T>
Node cos(T &&n) noexcept {
    return internal::UnaryOperator(MathOperator::MATH_COS, std::forward<T>(n));
}

template <typename T>
Node tan(T &&n) noexcept {
    return internal::UnaryOperator(MathOperator::MATH_TAN, std::forward<T>(n));
}

template <typename T>
Node asin(T &&n) noexcept {
    return internal::UnaryOperator(MathOperator::MATH_ARCSIN, std::forward<T>(n));
}

template <typename T>
Node acos(T &&n) noexcept {
    return internal::UnaryOperator(MathOperator::MATH_ARCCOS, std::forward<T>(n));
}

template <typename T>
Node atan(T &&n) noexcept {
    return internal::UnaryOperator(MathOperator::MATH_ARCTAN, std::forward<T>(n));
}

template <typename T>
Node sqrt(T &&n) noexcept {
    return internal::UnaryOperator(MathOperator::MATH_SQRT, std::forward<T>(n));
}

template <typename T>
Node log(T &&n) noexcept {
    return internal::UnaryOperator(MathOperator::MATH_LOG, std::forward<T>(n));
}

template <typename T>
Node log2(T &&n) noexcept {
    return internal::UnaryOperator(MathOperator::MATH_LOG2, std::forward<T>(n));
}

template <typename T>
Node log10(T &&n) noexcept {
    return internal::UnaryOperator(MathOperator::MATH_LOG10, std::forward<T>(n));
}

template <typename T>
Node exp(T &&n) noexcept {
    return internal::UnaryOperator(MathOperator::MATH_EXP, std::forward<T>(n));
}

} // namespace tomsolver
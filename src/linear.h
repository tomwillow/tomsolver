#pragma once

#include "mat.h"

namespace tomsolver {

/**
 * 求解线性方程组Ax = b。传入矩阵A，向量b，返回向量x。
 * @exception MathError 奇异矩阵
 * @exception MathError 矛盾方程组
 * @exception MathError 不定方程（设置GetConfig().allowIndeterminateEquation=true可以允许不定方程组返回一组特解）
 *
 */
Vec SolveLinear(const Mat &A, const Vec &b);

} // namespace tomsolver

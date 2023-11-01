#pragma once

#include "mat.h"

namespace tomsolver {

/**
 * 求解线性方程组Ax = b。传入矩阵A，向量b，返回向量x。
 * @exception MathError 奇异矩阵
 * @exception MathError 矛盾方程组
 * @exception MathError 不定方程（设置Config::get().allowIndeterminateEquation=true可以允许不定方程组返回一组特解）
 *
 */
Vec SolveLinear(Mat A, Vec b);

} // namespace tomsolver

#pragma once

#include "mat.h"
#include "symmat.h"

#include <functional>

namespace tomsolver {

/**
 * Armijo方法一维搜索，寻找alpha
 */
double Armijo(const Vec &x, const Vec &d, std::function<Vec(Vec)> f, std::function<Mat(Vec)> df, double uncert = 1e-5);

/**
 * 割线法 进行一维搜索，寻找alpha
 */
double FindAlpha(const Vec &x, const Vec &d, std::function<Vec(Vec)> f, double uncert = 1e-5);

/**
 * 解非线性方程组equations。
 * 初值及变量名通过varsTable传入。
 * @exception runtime_error 迭代次数超出限制
 */
Vec Solve(const std::unordered_map<std::string, double> &varsTable, const SymVec &equations);

} // namespace tomsolver

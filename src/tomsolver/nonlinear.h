#pragma once

#include "mat.h"
#include "symmat.h"
#include "vars_table.h"

#include <functional>

namespace tomsolver {

/**
 * Armijo方法一维搜索，寻找alpha
 */
double Armijo(const Vec &x, const Vec &d, std::function<Vec(Vec)> f, std::function<Mat(Vec)> df);

/**
 * 割线法 进行一维搜索，寻找alpha
 */
double FindAlpha(const Vec &x, const Vec &d, std::function<Vec(Vec)> f, double uncert = 1.0e-5);

/**
 * 解非线性方程组equations。
 * 初值及变量名通过varsTable传入。
 * @exception runtime_error 迭代次数超出限制
 */
VarsTable SolveByNewtonRaphson(const SymVec &equations, const VarsTable &varsTable);

/**
 * 解非线性方程组equations。
 * 初值及变量名通过varsTable传入。
 * @exception runtime_error 迭代次数超出限制
 */
VarsTable SolveByLM(const SymVec &equations, const VarsTable &varsTable);

/**
 * 解非线性方程组equations。
 * 初值及变量名通过varsTable传入。
 * @param equations 方程组。实质上是一个符号向量。
 * @param varsTable 初值表。
 * @exception runtime_error 迭代次数超出限制
 */
VarsTable Solve(const SymVec &equations, const VarsTable &varsTable);

/**
 * 解非线性方程组equations。
 * 变量名通过分析equations得到。初值通过Config::Get()得到。
 * @param equations 方程组。实质上是一个符号向量。
 * @exception runtime_error 迭代次数超出限制
 */
VarsTable Solve(const SymVec &equations);

} // namespace tomsolver

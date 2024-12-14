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
 * Solve a system of nonlinear equations.
 * Initial values and variable names are passed through varsTable. Will not use the Config::Get().initialValue
 * @param equations: The system of equations. Essentially, it is a symbolic vector.
 * @param varsTable: The table of initial values.
 * @throws tomsolver::MathError: If the number of iterations exceeds the limit.
 */
VarsTable Solve(const SymVec &equations, const VarsTable &varsTable);

/**
 * Solve the equations.
 * Variable names are obtained by analyzing the equations. Initial values are obtained through Config::Get().
 * @param equations: The system of equations. Essentially, it is a symbolic vector.
 * @throws tomsolver::MathError: If the number of iterations exceeds the limit.
 */
VarsTable Solve(const SymVec &equations);

} // namespace tomsolver

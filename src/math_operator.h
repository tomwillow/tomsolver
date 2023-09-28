#pragma once

#include "error_type.h"

#include <string>
#define _USE_MATH_DEFINES
#include <math.h>

#include <limits>

namespace tomsolver {

constexpr double PI = M_PI;

constexpr double eps = std::numeric_limits<double>::epsilon();

enum class MathOperator {
    MATH_NULL,
    //一元
    MATH_POSITIVE,
    MATH_NEGATIVE,

    //函数
    MATH_SIN,
    MATH_COS,
    MATH_TAN,
    MATH_ARCSIN,
    MATH_ARCCOS,
    MATH_ARCTAN,
    MATH_SQRT,
    MATH_LOG,
    MATH_LOG2,
    MATH_LOG10,
    MATH_EXP,

    //二元
    MATH_ADD,
    MATH_SUB,
    MATH_MULTIPLY,
    MATH_DIVIDE,
    MATH_POWER,
    MATH_AND,
    MATH_OR,
    MATH_MOD,

    MATH_LEFT_PARENTHESIS,
    MATH_RIGHT_PARENTHESIS
};

/**
 * 操作符转std::string
 */
std::string MathOperatorToStr(MathOperator op);

/**
 * 取得操作数的数量。
 */
int GetOperatorNum(MathOperator op) noexcept;

/**
* 返回运算符的优先级

*/
int Rank(MathOperator op) noexcept;

/**
 * 返回运算符结合性
 */
bool IsLeft2Right(MathOperator eOperator) noexcept;

/**
 * 返回是否满足交换律
 */
bool InAssociativeLaws(MathOperator eOperator) noexcept;

/**
 * 是整数 且 为偶数
 * FIXME: 超出long long范围的处理
 */
bool IsIntAndEven(double n) noexcept;

double Calc(MathOperator op, double v1, double v2);

} // namespace tomsolver
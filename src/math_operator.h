#pragma once

#include "error_type.h"

#include <string>
#include <cassert>

namespace tomsolver {

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
    MATH_LN,
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
std::string MathOperatorToStr(MathOperator op) {
    switch (op) {
    case MathOperator::MATH_NULL:
        assert(0);
        return "";
    //一元
    case MathOperator::MATH_POSITIVE:
        return "+";
    case MathOperator::MATH_NEGATIVE:
        return "-";
    //函数
    case MathOperator::MATH_SIN:
        return "sin";
    case MathOperator::MATH_COS:
        return "cos";
    case MathOperator::MATH_TAN:
        return "tan";
    case MathOperator::MATH_ARCSIN:
        return "asin";
    case MathOperator::MATH_ARCCOS:
        return "acos";
    case MathOperator::MATH_ARCTAN:
        return "atan";
    case MathOperator::MATH_SQRT:
        return "sqrt";
    case MathOperator::MATH_LN:
        return "ln";
    case MathOperator::MATH_LOG10:
        return "log10";
    case MathOperator::MATH_EXP:
        return "exp";
    //二元
    case MathOperator::MATH_ADD:
        return "+";
    case MathOperator::MATH_SUB:
        return "-";
    case MathOperator::MATH_MULTIPLY:
        return "*";
    case MathOperator::MATH_DIVIDE:
        return "/";
    case MathOperator::MATH_POWER:
        return "^";
    case MathOperator::MATH_AND:
        return "&";
    case MathOperator::MATH_OR:
        return "|";
    case MathOperator::MATH_MOD:
        return "%";
    case MathOperator::MATH_LEFT_PARENTHESIS:
        return "(";
    case MathOperator::MATH_RIGHT_PARENTHESIS:
        return ")";
    }
    throw MathError{ErrorType::ERROR_WRONG_MATH_OPERATOR, std::string("value=" + std::to_string(static_cast<int>(op)))};
}

/**
 * 取得操作数的数量。
 */
int GetOperatorNum(MathOperator op) {
    switch (op) {
    case MathOperator::MATH_POSITIVE: //正负号
    case MathOperator::MATH_NEGATIVE:

    case MathOperator::MATH_SIN:
    case MathOperator::MATH_COS:
    case MathOperator::MATH_TAN:
    case MathOperator::MATH_ARCSIN:
    case MathOperator::MATH_ARCCOS:
    case MathOperator::MATH_ARCTAN:
    case MathOperator::MATH_SQRT:
    case MathOperator::MATH_LN:
    case MathOperator::MATH_LOG10:
    case MathOperator::MATH_EXP:
        return 1;

    case MathOperator::MATH_ADD:
    case MathOperator::MATH_SUB:
    case MathOperator::MATH_MULTIPLY:
    case MathOperator::MATH_DIVIDE:
    case MathOperator::MATH_POWER: //^
    case MathOperator::MATH_AND:   //&
    case MathOperator::MATH_OR:    //|
    case MathOperator::MATH_MOD:   //%
        return 2;

    case MathOperator::MATH_LEFT_PARENTHESIS:
    case MathOperator::MATH_RIGHT_PARENTHESIS:
        assert(0);
        return 0;
    }
    assert(0);
    return 0;
}

/**
* 返回运算符的优先级

*/
int Rank(MathOperator op) {
    switch (op) {
    case MathOperator::MATH_SIN:
    case MathOperator::MATH_COS:
    case MathOperator::MATH_TAN:
    case MathOperator::MATH_ARCSIN:
    case MathOperator::MATH_ARCCOS:
    case MathOperator::MATH_ARCTAN:
    case MathOperator::MATH_SQRT:
    case MathOperator::MATH_LN:
    case MathOperator::MATH_LOG10:
    case MathOperator::MATH_EXP:
        return 15;

    case MathOperator::MATH_POSITIVE: //除了函数，所有运算符均可将正负号挤出
    case MathOperator::MATH_NEGATIVE:
        return 14;

    case MathOperator::MATH_MOD: //%
        return 13;

    case MathOperator::MATH_AND: //&
    case MathOperator::MATH_OR:  //|
        return 12;

    case MathOperator::MATH_POWER: //^
        return 11;

    case MathOperator::MATH_MULTIPLY:
    case MathOperator::MATH_DIVIDE:
        return 10;

    case MathOperator::MATH_ADD:
    case MathOperator::MATH_SUB:
        return 5;

    case MathOperator::MATH_LEFT_PARENTHESIS: //左右括号优先级小是为了不被其余任何运算符挤出
    case MathOperator::MATH_RIGHT_PARENTHESIS:
        return 0;
    }
    throw MathError{ErrorType::ERROR_WRONG_MATH_OPERATOR, std::string("value=" + std::to_string(static_cast<int>(op)))};
}

/**
 * 返回运算符结合性
 */
bool IsLeft2Right(MathOperator eOperator) {
    switch (eOperator) {
    case MathOperator::MATH_MOD: //%
    case MathOperator::MATH_AND: //&
    case MathOperator::MATH_OR:  //|
    case MathOperator::MATH_MULTIPLY:
    case MathOperator::MATH_DIVIDE:
    case MathOperator::MATH_ADD:
    case MathOperator::MATH_SUB:
        return true;

    case MathOperator::MATH_POSITIVE: //正负号为右结合
    case MathOperator::MATH_NEGATIVE:
    case MathOperator::MATH_POWER: //^
        return false;
        //函数和括号不计结合性
    }
    assert(0);
    return false;
}

/**
 * 返回是否满足交换律
 */
bool InAssociativeLaws(MathOperator eOperator) {
    switch (eOperator) {
    case MathOperator::MATH_SQRT:
    case MathOperator::MATH_SIN:
    case MathOperator::MATH_COS:
    case MathOperator::MATH_TAN:
    case MathOperator::MATH_ARCSIN:
    case MathOperator::MATH_ARCCOS:
    case MathOperator::MATH_ARCTAN:
    case MathOperator::MATH_LN:
    case MathOperator::MATH_LOG10:
    case MathOperator::MATH_EXP:

    case MathOperator::MATH_POSITIVE: //正负号
    case MathOperator::MATH_NEGATIVE:

    case MathOperator::MATH_MOD:   //%
    case MathOperator::MATH_AND:   //&
    case MathOperator::MATH_OR:    //|
    case MathOperator::MATH_POWER: //^
    case MathOperator::MATH_DIVIDE:
    case MathOperator::MATH_SUB:

    case MathOperator::MATH_LEFT_PARENTHESIS:
    case MathOperator::MATH_RIGHT_PARENTHESIS:
        return false;

    case MathOperator::MATH_ADD:
    case MathOperator::MATH_MULTIPLY:
        return true;
    }
    assert(0);
    return false;
}

} // namespace tomsolver
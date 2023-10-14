#include "math_operator.h"
#include "config.h"

#include <limits>
#include <cassert>
#include <cmath>

namespace tomsolver {

std::string MathOperatorToStr(MathOperator op) {
    switch (op) {
    case MathOperator::MATH_NULL:
        assert(0);
        return "";
    // 一元
    case MathOperator::MATH_POSITIVE:
        return "+";
    case MathOperator::MATH_NEGATIVE:
        return "-";
    // 函数
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
    case MathOperator::MATH_LOG:
        return "log";
    case MathOperator::MATH_LOG2:
        return "log2";
    case MathOperator::MATH_LOG10:
        return "log10";
    case MathOperator::MATH_EXP:
        return "exp";
    // 二元
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
    assert(0);
    return "err";
}

int GetOperatorNum(MathOperator op) noexcept {
    switch (op) {
    case MathOperator::MATH_POSITIVE: // 正负号
    case MathOperator::MATH_NEGATIVE:

    case MathOperator::MATH_SIN:
    case MathOperator::MATH_COS:
    case MathOperator::MATH_TAN:
    case MathOperator::MATH_ARCSIN:
    case MathOperator::MATH_ARCCOS:
    case MathOperator::MATH_ARCTAN:
    case MathOperator::MATH_SQRT:
    case MathOperator::MATH_LOG:
    case MathOperator::MATH_LOG2:
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
        break;
    default:
        assert(0);
        break;
    }
    assert(0);
    return 0;
}

int Rank(MathOperator op) noexcept {
    switch (op) {
    case MathOperator::MATH_SIN:
    case MathOperator::MATH_COS:
    case MathOperator::MATH_TAN:
    case MathOperator::MATH_ARCSIN:
    case MathOperator::MATH_ARCCOS:
    case MathOperator::MATH_ARCTAN:
    case MathOperator::MATH_SQRT:
    case MathOperator::MATH_LOG:
    case MathOperator::MATH_LOG2:
    case MathOperator::MATH_LOG10:
    case MathOperator::MATH_EXP:
        return 15;

    case MathOperator::MATH_POSITIVE: // 除了函数，所有运算符均可将正负号挤出
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

    case MathOperator::MATH_LEFT_PARENTHESIS: // 左右括号优先级小是为了不被其余任何运算符挤出
    case MathOperator::MATH_RIGHT_PARENTHESIS:
        return 0;
    default:
        assert(0);
        break;
    }
    assert(0);
    return 0;
}

bool IsLeft2Right(MathOperator eOperator) noexcept {
    switch (eOperator) {
    case MathOperator::MATH_MOD: //%
    case MathOperator::MATH_AND: //&
    case MathOperator::MATH_OR:  //|
    case MathOperator::MATH_MULTIPLY:
    case MathOperator::MATH_DIVIDE:
    case MathOperator::MATH_ADD:
    case MathOperator::MATH_SUB:
        return true;

    case MathOperator::MATH_POSITIVE: // 正负号为右结合
    case MathOperator::MATH_NEGATIVE:
    case MathOperator::MATH_POWER: //^
        return false;

    // 函数和括号不计结合性
    case MathOperator::MATH_SIN:
    case MathOperator::MATH_COS:
    case MathOperator::MATH_TAN:
    case MathOperator::MATH_ARCSIN:
    case MathOperator::MATH_ARCCOS:
    case MathOperator::MATH_ARCTAN:
    case MathOperator::MATH_SQRT:
    case MathOperator::MATH_LOG:
    case MathOperator::MATH_LOG2:
    case MathOperator::MATH_LOG10:
    case MathOperator::MATH_EXP:

    case MathOperator::MATH_LEFT_PARENTHESIS:
    case MathOperator::MATH_RIGHT_PARENTHESIS:
        return true;
    default:
        assert(0);
    }
    return false;
}

bool InAssociativeLaws(MathOperator eOperator) noexcept {
    switch (eOperator) {

    case MathOperator::MATH_POSITIVE: // 正负号
    case MathOperator::MATH_NEGATIVE:

    case MathOperator::MATH_SQRT:
    case MathOperator::MATH_SIN:
    case MathOperator::MATH_COS:
    case MathOperator::MATH_TAN:
    case MathOperator::MATH_ARCSIN:
    case MathOperator::MATH_ARCCOS:
    case MathOperator::MATH_ARCTAN:
    case MathOperator::MATH_LOG:
    case MathOperator::MATH_LOG2:
    case MathOperator::MATH_LOG10:
    case MathOperator::MATH_EXP:

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
    default:
        assert(0);
        break;
    }
    assert(0);
    return false;
}

bool IsFunction(MathOperator op) noexcept {
    switch (op) {
    case MathOperator::MATH_SIN:
    case MathOperator::MATH_COS:
    case MathOperator::MATH_TAN:
    case MathOperator::MATH_ARCSIN:
    case MathOperator::MATH_ARCCOS:
    case MathOperator::MATH_ARCTAN:
    case MathOperator::MATH_SQRT:
    case MathOperator::MATH_LOG:
    case MathOperator::MATH_LOG2:
    case MathOperator::MATH_LOG10:
    case MathOperator::MATH_EXP:
        return true;

    case MathOperator::MATH_POSITIVE:
    case MathOperator::MATH_NEGATIVE:
    case MathOperator::MATH_MOD:   //%
    case MathOperator::MATH_AND:   //&
    case MathOperator::MATH_OR:    //|
    case MathOperator::MATH_POWER: //^
    case MathOperator::MATH_MULTIPLY:
    case MathOperator::MATH_DIVIDE:
    case MathOperator::MATH_ADD:
    case MathOperator::MATH_SUB:
    case MathOperator::MATH_LEFT_PARENTHESIS:
    case MathOperator::MATH_RIGHT_PARENTHESIS:
        return false;
    default:
        assert(0);
        break;
    }
    assert(0);
    return false;
}

double Calc(MathOperator op, double v1, double v2) {
    double ret = std::numeric_limits<double>::quiet_NaN();
    switch (op) {
    case MathOperator::MATH_SIN:
        ret = std::sin(v1);
        break;
    case MathOperator::MATH_COS:
        ret = std::cos(v1);
        break;
    case MathOperator::MATH_TAN:
        ret = std::tan(v1);
        break;
    case MathOperator::MATH_ARCSIN:
        ret = std::asin(v1);
        break;
    case MathOperator::MATH_ARCCOS:
        ret = std::acos(v1);
        break;
    case MathOperator::MATH_ARCTAN:
        ret = std::atan(v1);
        break;
    case MathOperator::MATH_SQRT:
        ret = std::sqrt(v1);
        break;
    case MathOperator::MATH_LOG:
        ret = std::log(v1);
        break;
    case MathOperator::MATH_LOG2:
        ret = std::log2(v1);
        break;
    case MathOperator::MATH_LOG10:
        ret = std::log10(v1);
        break;
    case MathOperator::MATH_EXP:
        ret = std::exp(v1);
        break;
    case MathOperator::MATH_POSITIVE:
        ret = v1;
        break;
    case MathOperator::MATH_NEGATIVE:
        ret = -v1;
        break;

    case MathOperator::MATH_MOD: //%
        ret = (int)v1 % (int)v2;
        break;
    case MathOperator::MATH_AND: //&
        ret = (int)v1 & (int)v2;
        break;
    case MathOperator::MATH_OR: //|
        ret = (int)v1 | (int)v2;
        break;

    case MathOperator::MATH_POWER: //^
        ret = std::pow(v1, v2);
        break;

    case MathOperator::MATH_ADD:
        ret = v1 + v2;
        break;
    case MathOperator::MATH_SUB:
        ret = v1 - v2;
        break;
    case MathOperator::MATH_MULTIPLY:
        ret = v1 * v2;
        break;
    case MathOperator::MATH_DIVIDE:
        ret = v1 / v2;
        break;
    default:
        assert(0 && "[Calc] bug.");
        break;
    }

    if (GetConfig().throwOnInvalidValue == false) {
        return ret;
    }

    bool isInvalid = (ret == std::numeric_limits<double>::infinity()) ||
                     (ret == -std::numeric_limits<double>::infinity()) || (ret != ret);
    if (isInvalid) {
        std::string info;
        info = "expression: \"";
        switch (GetOperatorNum(op)) {
        case 1:
            info += MathOperatorToStr(op) + " " + ToString(v1);
            break;
        case 2:
            info += ToString(v1) + " " + MathOperatorToStr(op) + " " + ToString(v2);
            break;
        default:
            assert(0);
        }
        info += "\"";
        throw MathError(ErrorType::ERROR_INVALID_NUMBER, info);
    }

    return ret;
}

} // namespace tomsolver
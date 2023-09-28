#include "math_operator.h"
#include "config.h"

#include <cassert>

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
    throw MathError{ErrorType::ERROR_WRONG_MATH_OPERATOR, std::string("value=" + std::to_string(static_cast<int>(op)))};
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
        return 0;
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
    }
    assert(0);
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
    }
    assert(0);
    return false;
}

bool IsIntAndEven(double n) noexcept {
    long long i = (long long)n;
    if (abs(n - i) <= eps)
        if (i % 2 == 0)
            return true;
    return false;
}

double Calc(MathOperator op, double v1, double v2) {
    double ret = std::numeric_limits<double>::quiet_NaN();
    switch (op) {
    case MathOperator::MATH_SIN:
        ret = sin(v1);
        break;
    case MathOperator::MATH_COS:
        ret = cos(v1);
        break;
    case MathOperator::MATH_TAN: {
        // x!=k*pi+pi/2 -> 2*x/pi != 2*k+1(odd)
        double value = v1 * 2.0 / PI;
        if (abs(value - (int)value) < eps && (int)value % 2 != 1) {
            throw MathError{ErrorType::ERROR_OUTOF_DOMAIN,
                            std::string("tan(") + std::to_string(value) + std::string("")};
        }
        ret = tan(v1);
        break;
    }
    case MathOperator::MATH_ARCSIN:
        if (v1 < -1.0 || v1 > 1.0) {
            throw MathError{ErrorType::ERROR_OUTOF_DOMAIN,
                            std::string("arcsin(") + std::to_string(v1) + std::string("")};
        }
        ret = asin(v1);
        break;
    case MathOperator::MATH_ARCCOS:
        if (v1 < -1.0 || v1 > 1.0) {
            throw MathError{ErrorType::ERROR_OUTOF_DOMAIN,
                            std::string("arccos(") + std::to_string(v1) + std::string("")};
        }
        ret = acos(v1);
        break;
    case MathOperator::MATH_ARCTAN:
        ret = atan(v1);
        break;
    case MathOperator::MATH_SQRT:
        if (v1 < 0.0) {
            throw MathError(ErrorType::ERROR_I, std::string("sqrt(") + std::to_string(v1) + std::string(")"));
        }
        ret = sqrt(v1);
        break;
    case MathOperator::MATH_LOG:
        if (v1 <= 0) {
            throw MathError{ErrorType::ERROR_OUTOF_DOMAIN, std::string("log(") + std::to_string(v1) + std::string("")};
        }
        ret = log(v1);
        break;
    case MathOperator::MATH_LOG2:
        if (v1 <= 0) {
            throw MathError{ErrorType::ERROR_OUTOF_DOMAIN, std::string("log2(") + std::to_string(v1) + std::string("")};
        }
        ret = log2(v1);
        break;
    case MathOperator::MATH_LOG10:
        if (v1 <= 0) // log(0)或log(负数)
        {
            throw MathError{ErrorType::ERROR_OUTOF_DOMAIN,
                            std::string("log10(") + std::to_string(v1) + std::string("")};
        }
        ret = log10(v1);
        break;
    case MathOperator::MATH_EXP:
        ret = exp(v1);
        break;
    case MathOperator::MATH_POSITIVE:
        break;
    case MathOperator::MATH_NEGATIVE:
        ret = -v1;
        break;

    case MathOperator::MATH_MOD: //%
        if ((int)v2 == 0)
            throw MathError{ErrorType::ERROR_DIVIDE_ZERO, std::to_string(v2)};
        ret = (int)v1 % (int)v2;
        break;
    case MathOperator::MATH_AND: //&
        ret = (int)v1 & (int)v2;
        break;
    case MathOperator::MATH_OR: //|
        ret = (int)v1 | (int)v2;
        break;

    case MathOperator::MATH_POWER: //^
        // 0^0
        if (abs(v1) < eps && abs(v2) < eps) {
            throw MathError{ErrorType::ERROR_ZERO_POWEROF_ZERO, ""};
        }

        //(-1)^0.5=i
        if (v1 < 0 && IsIntAndEven(1.0 / v2)) {
            throw MathError{ErrorType::ERROR_I,
                            std::string("pow(") + std::to_string(v1) + "," + std::to_string(v2) + ""};
        }
        ret = pow(v1, v2);
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
        if (GetConfig().checkDomain && abs(v2) == 0) {
            throw MathError{ErrorType::ERROR_DIVIDE_ZERO, ""};
        }
        ret = v1 / v2;
        break;
    default:
        assert(0 && "[Calc] bug.");
        break;
    }

    return ret;
}

} // namespace tomsolver
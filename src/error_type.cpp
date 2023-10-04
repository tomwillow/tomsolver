#include "error_type.h"

#include <cassert>

namespace tomsolver {

std::string GetErrorInfo(ErrorType err) {
    switch (err) {
    case ErrorType::ERROR_INVALID_NUMBER:
        return u8"invalid number";
        break;
    case ErrorType::ERROR_ILLEGALCHAR:
        return u8"illegal character";
        break;
    case ErrorType::ERROR_PARENTHESIS_NOT_MATCH:
        return u8"括号不匹配。";
        break;
    case ErrorType::ERROR_INVALID_VARNAME:
        return u8"不正确的变量名（必须以下划线\"_\"或英文字母开头）。";
        break;
    case ErrorType::ERROR_WRONG_EXPRESSION:
        return u8"错误的表达式。";
        break;
    case ErrorType::ERROR_EMPTY_INPUT:
        return u8"表达式为空。";
        break;
    case ErrorType::ERROR_UNDEFINED_VARIABLE:
        return u8"未定义的变量。";
        break;
    case ErrorType::ERROR_SUBS_NOT_EQUAL:
        return u8"替换与被替换数目不等。";
        break;
    case ErrorType::ERROR_NOT_LINK_VARIABLETABLE:
        return u8"程序未链接变量表。";
        break;
    case ErrorType::ERROR_OUTOF_DOMAIN:
        return u8"超出定义域。";
        break;
    case ErrorType::ERROR_VAR_COUNT_NOT_EQUAL_NUM_COUNT:
        return u8"变量名与初始值数量不对等。";
        break;
    case ErrorType::ERROR_VAR_HAS_BEEN_DEFINED:
        return u8"变量重定义";
        break;
    case ErrorType::ERROR_INDETERMINATE_EQUATION:
        return u8"不定方程";
        break;
    case ErrorType::ERROR_SINGULAR_MATRIX:
        return u8"矩阵奇异";
        break;
    case ErrorType::ERROR_INFINITY_SOLUTIONS:
        return u8"无穷多解";
        break;
    case ErrorType::ERROR_OVER_DETERMINED_EQUATIONS:
        return u8"方程组过定义";
        break;
    default:
        assert(0);
        break;
    }
    return u8"GetErrorInfo: bug";
}

const char *MathError::what() const noexcept {
    return errInfo.c_str();
}

} // namespace tomsolver
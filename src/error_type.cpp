#include "error_type.h"

#include <cassert>

namespace tomsolver {

std::string GetErrorInfo(ErrorType err) {
    switch (err) {
    case ErrorType::ERROR_ILLEGALCHAR:
        return u8"错误：出现非法字符。";
        break;
    case ErrorType::ERROR_PARENTHESIS_NOT_MATCH:
        return u8"错误：括号不匹配。";
        break;
    case ErrorType::ERROR_INVALID_VARNAME:
        return u8"错误：不正确的变量名（必须以下划线\"_\"或英文字母开头）。";
        break;
    case ErrorType::ERROR_WRONG_EXPRESSION:
        return u8"错误：错误的表达式。";
        break;
    case ErrorType::ERROR_EMPTY_INPUT:
        return u8"表达式为空。";
        break;
    case ErrorType::ERROR_DIVIDE_ZERO:
        return u8"错误：不能除以0。";
        break;
    case ErrorType::ERROR_UNDEFINED_VARIABLE:
        return u8"错误：未定义的变量。";
        break;
    case ErrorType::ERROR_ZERO_POWEROF_ZERO:
        return u8"错误：0的0次方。";
        break;
    case ErrorType::ERROR_SUBS_NOT_EQUAL:
        return u8"错误：替换与被替换数目不等。";
        break;
    case ErrorType::ERROR_NOT_LINK_VARIABLETABLE:
        return u8"程序错误：未链接变量表。";
        break;
    case ErrorType::ERROR_OUTOF_DOMAIN:
        return u8"错误：超出定义域。";
        break;
    case ErrorType::ERROR_VAR_COUNT_NOT_EQUAL_NUM_COUNT:
        return u8"错误：变量名与初始值数量不对等。";
        break;
    case ErrorType::ERROR_I:
        return u8"暂不支持虚数。";
        break;
    default:
        assert(0);
        break;
    }
    return u8"GetErrorInfo: bug";
}


const char *MathError::what() const
{
    return errInfo.c_str();
}

} // namespace tomsolver
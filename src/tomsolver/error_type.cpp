#include "error_type.h"

#include <cassert>
#include <sstream>

namespace tomsolver {

std::string GetErrorInfo(ErrorType err) {
    switch (err) {
    case ErrorType::ERROR_INVALID_NUMBER:
        return u8"invalid number";
        break;
    case ErrorType::ERROR_ILLEGALCHAR:
        return u8"illegal character";
        break;
    case ErrorType::ERROR_INVALID_VARNAME:
        return u8"invalid variable name (must start with an underscore \"_\" or a letter)";
        break;
    case ErrorType::ERROR_WRONG_EXPRESSION:
        return u8"invalid expression";
        break;
    case ErrorType::ERROR_EMPTY_INPUT:
        return u8"empty input";
        break;
    case ErrorType::ERROR_UNDEFINED_VARIABLE:
        return u8"undefined variable";
        break;
    case ErrorType::ERROR_SUBS_NOT_EQUAL:
        return u8"number of substitutions does not match the number of items to be replaced";
        break;
    case ErrorType::ERROR_NOT_LINK_VARIABLETABLE:
        return u8"not linked variable table";
        break;
    case ErrorType::ERROR_OUTOF_DOMAIN:
        return u8"out of domain";
        break;
    case ErrorType::ERROR_VAR_COUNT_NOT_EQUAL_NUM_COUNT:
        return u8"the number of variable is not equal with number count";
        break;
    case ErrorType::ERROR_VAR_HAS_BEEN_DEFINED:
        return u8"variable redefined";
        break;
    case ErrorType::ERROR_INDETERMINATE_EQUATION:
        return u8"indeterminate equation";
        break;
    case ErrorType::ERROR_SINGULAR_MATRIX:
        return u8"singular matrix";
        break;
    case ErrorType::ERROR_INFINITY_SOLUTIONS:
        return u8"infinite solutions";
        break;
    case ErrorType::ERROR_OVER_DETERMINED_EQUATIONS:
        return u8"overdetermined equations";
        break;
    case ErrorType::SIZE_NOT_MATCH:
        return u8"size does not match";
    default:
        assert(0);
        break;
    }
    return u8"GetErrorInfo: bug";
}

MathError::MathError(ErrorType errorType, const std::string &extInfo)
    : std::runtime_error(extInfo), errorType(errorType) {
    errInfo = GetErrorInfo(errorType);
    if (!extInfo.empty()) {
        errInfo += ": \"" + extInfo + "\"";
    }
}

const char *MathError::what() const noexcept {
    return errInfo.c_str();
}

ErrorType MathError::GetErrorType() const noexcept {
    return errorType;
}

} // namespace tomsolver
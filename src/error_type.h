#pragma once

#include <string>

namespace tomsolver {

enum class ErrorType {
    ERROR_INVALID_NUMBER,                // 出现无效的浮点数(inf, -inf, nan)
    ERROR_ILLEGALCHAR,                   //出现非法字符
    ERROR_PARENTHESIS_NOT_MATCH,         //括号不匹配
    ERROR_INVALID_VARNAME,               //无效变量名
    ERROR_WRONG_EXPRESSION,              //表达式逻辑不正确
    ERROR_EMPTY_INPUT,                   //表达式为空
    ERROR_UNDEFINED_VARIABLE,            //未定义的变量
    ERROR_SUBS_NOT_EQUAL,                //替换与被替换数量不对等
    ERROR_NOT_LINK_VARIABLETABLE,        //未链接变量表
    ERROR_OUTOF_DOMAIN,                  //计算超出定义域
    ERROR_VAR_COUNT_NOT_EQUAL_NUM_COUNT, //定义变量时变量数量与初始值不等
    ERROR_VAR_HAS_BEEN_DEFINED,          //变量重定义
    ERROR_INDETERMINATE_EQUATION,        //不定方程
    ERROR_SINGULAR_MATRIX,               //矩阵奇异
    ERROR_INFINITY_SOLUTIONS,            //无穷多解
    ERROR_OVER_DETERMINED_EQUATIONS      //方程组过定义
};

std::string GetErrorInfo(ErrorType err);

class MathError : public std::exception {
public:
    MathError(ErrorType errorType, const std::string &extInfo)
        : errorType(errorType), errInfo(GetErrorInfo(errorType) + " : " + extInfo) {}

    virtual const char *what() const noexcept override;

private:
    ErrorType errorType;
    std::string errInfo;
};

} // namespace tomsolver
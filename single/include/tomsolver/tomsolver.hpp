#pragma once

#define _USE_MATH_DEFINES
#include <algorithm>
#include <array>
#include <cassert>
#include <cctype>
#include <cmath>
#include <forward_list>
#include <functional>
#include <gmock/gmock.h>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <math.h>
#include <memory>
#include <pthread.h>
#include <queue>
#include <regex>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <valarray>
#include <vector>

namespace tomsolver {

enum class ErrorType {
    ERROR_INVALID_NUMBER,                // 出现无效的浮点数(inf, -inf, nan)
    ERROR_ILLEGALCHAR,                   // 出现非法字符
    ERROR_INVALID_VARNAME,               // 无效变量名
    ERROR_WRONG_EXPRESSION,              // 表达式逻辑不正确
    ERROR_EMPTY_INPUT,                   // 表达式为空
    ERROR_UNDEFINED_VARIABLE,            // 未定义的变量
    ERROR_SUBS_NOT_EQUAL,                // 替换与被替换数量不对等
    ERROR_NOT_LINK_VARIABLETABLE,        // 未链接变量表
    ERROR_OUTOF_DOMAIN,                  // 计算超出定义域
    ERROR_VAR_COUNT_NOT_EQUAL_NUM_COUNT, // 定义变量时变量数量与初始值不等
    ERROR_VAR_HAS_BEEN_DEFINED,          // 变量重定义
    ERROR_INDETERMINATE_EQUATION,        // 不定方程
    ERROR_SINGULAR_MATRIX,               // 矩阵奇异
    ERROR_INFINITY_SOLUTIONS,            // 无穷多解
    ERROR_OVER_DETERMINED_EQUATIONS,     // 方程组过定义
    SIZE_NOT_MATCH                       // 维数不匹配
};

std::string GetErrorInfo(ErrorType err);

class MathError : public std::exception {
public:
    MathError(ErrorType errorType, std::string_view extInfo = {});

    virtual const char *what() const noexcept override;

    ErrorType GetErrorType() const noexcept;

private:
    ErrorType errorType;
    std::string errInfo;
};

} // namespace tomsolver

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
    case ErrorType::SIZE_NOT_MATCH:
        return u8"size not match";
    default:
        assert(0);
        break;
    }
    return u8"GetErrorInfo: bug";
}

MathError::MathError(ErrorType errorType, std::string_view extInfo) : errorType(errorType) {
    std::stringstream ss;
    ss << GetErrorInfo(errorType) << ": \"" << extInfo << "\"";
    errInfo = ss.str();
}

const char *MathError::what() const noexcept {
    return errInfo.c_str();
}

ErrorType MathError::GetErrorType() const noexcept {
    return errorType;
}

} // namespace tomsolver

namespace tomsolver {

enum class LogLevel { OFF, FATAL, ERROR, WARN, INFO, DEBUG, TRACE, ALL };

enum class NonlinearMethod { NEWTON_RAPHSON, LM };

struct Config {
    /**
     * 指定出现浮点数无效值(inf, -inf, nan)时，是否抛出异常。默认为true。
     */
    bool throwOnInvalidValue = true;

    double epsilon = 1.0e-9;

    LogLevel logLevel = LogLevel::WARN;

    /**
     * 最大迭代次数限制
     */
    int maxIterations = 100;

    /**
     * 求解方法
     */
    NonlinearMethod nonlinearMethod = NonlinearMethod::NEWTON_RAPHSON;

    /**
     * 非线性方程求解时，当没有为VarsTable传初值时，设定的初值
     */
    double initialValue = 1.0;

    /**
     * 是否允许不定方程存在。
     * 例如，当等式数量大于未知数数量时，方程组成为不定方程；
     * 如果允许，此时将返回一组特解；如果不允许，将抛出异常。
     */
    bool allowIndeterminateEquation = false;

    void Reset() noexcept;

    template <auto = 0>
    static Config &get() {
        static Config config;
        return config;
    }

private:
    Config() = default;
};

std::string ToString(double value) noexcept;

} // namespace tomsolver

namespace tomsolver {

std::string ToString(double value) noexcept {
    static const std::array strategy = {
        std::tuple{"%.16e", std::regex{"\\.?0+(?=e)"}},
        std::tuple{"%.16f", std::regex{"\\.?0+(?=$)"}},
    };

    if (value == 0.0) {
        return "0";
    }

    char buf[64];

    // 绝对值过大 或者 绝对值过小，应该使用科学计数法来表示
    auto getStrategyIdx = [absValue = std::abs(value)] {
        return (absValue >= 1.0e16 || absValue <= 1.0e-16) ? 0 : 1;
    };

    auto &[fmt, re] = strategy[getStrategyIdx()];

    sprintf(buf, fmt, value);
    return std::regex_replace(buf, re, "");
}

void Config::Reset() noexcept {
    *this = {};
}

} // namespace tomsolver

namespace tomsolver {

constexpr double PI = M_PI;

template <typename T>
T radians(T &&t) noexcept {
    return std::forward<T>(t) / 180.0 * PI;
}

template <typename T>
T degrees(T &&t) noexcept {
    return std::forward<T>(t) * 180.0 / PI;
}

enum class MathOperator {
    MATH_NULL,
    // 一元
    MATH_POSITIVE,
    MATH_NEGATIVE,

    // 函数
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

    // 二元
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
 * 返回是否是函数
 */
bool IsFunction(MathOperator op) noexcept;

/**
 * 是整数 且 为偶数
 * FIXME: 超出long long范围的处理
 */
bool IsIntAndEven(double n) noexcept;

double Calc(MathOperator op, double v1, double v2);

} // namespace tomsolver

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

    if (Config::get().throwOnInvalidValue == false) {
        return ret;
    }

    bool isInvalid = (ret == std::numeric_limits<double>::infinity()) ||
                     (ret == -std::numeric_limits<double>::infinity()) || (ret != ret);
    if (isInvalid) {
        // std::string info;
        std::stringstream info;
        info << "expression: \"";
        switch (GetOperatorNum(op)) {
        case 1:
            info << MathOperatorToStr(op) << " " << ToString(v1);
            break;
        case 2:
            info << ToString(v1) << " " << MathOperatorToStr(op) << " " << ToString(v2);
            break;
        default:
            assert(0);
        }
        info << "\"";
        throw MathError(ErrorType::ERROR_INVALID_NUMBER, info.str());
    }

    return ret;
}

} // namespace tomsolver
/*

Original Inverse(), Adjoint(), GetCofactor(), Det() is from https://github.com/taehwan642:

///////////////////////////////////////////
    MADE BY TAE HWAN KIM, SHIN JAE HO
    김태환, 신재호 제작
    If you see this documents, you can learn & understand Faster.
    밑에 자료들을 보시면, 더욱 빠르게 배우고 이해하실 수 있으실겁니다.
    https://www.wikihow.com/Find-the-Inverse-of-a-3x3-Matrix
    https://www.wikihow.com/Find-the-Determinant-of-a-3X3-Matrix
    LAST UPDATE 2020 - 03 - 30
    마지막 업데이트 2020 - 03 - 30
    This is my Github Profile. You can use this source whenever you want.
    제 깃허브 페이지입니다. 언제든지 이 소스를 가져다 쓰셔도 됩니다.
    https://github.com/taehwan642
    Thanks :)
    감사합니다 :)
///////////////////////////////////////////

*/

namespace tomsolver {

class Vec;

class Mat {
public:
    explicit Mat(int row, int col, double initValue = 0) noexcept;

    Mat(std::initializer_list<std::initializer_list<double>> init) noexcept;

    Mat(int row, int col, std::valarray<double> data) noexcept;

    Mat(const Mat &) = default;
    Mat(Mat &&) = default;
    Mat &operator=(const Mat &) = default;
    Mat &operator=(Mat &&) = default;

    std::slice_array<double> Row(int i, int offset = 0);
    std::slice_array<double> Col(int j, int offset = 0);
    auto Row(int i, int offset = 0) const -> decltype(std::declval<const std::valarray<double>>()[std::slice{}]);
    auto Col(int j, int offset = 0) const -> decltype(std::declval<const std::valarray<double>>()[std::slice{}]);
    const double &Value(int i, int j) const;
    double &Value(int i, int j);

    bool operator==(double m) const noexcept;
    bool operator==(const Mat &b) const noexcept;

    // be negative
    Mat operator-() noexcept;

    Mat operator+(const Mat &b) const noexcept;
    Mat &operator+=(const Mat &b) noexcept;

    Mat operator-(const Mat &b) const noexcept;

    Mat operator*(double m) const noexcept;
    Mat operator*(const Mat &b) const noexcept;

    int Rows() const noexcept;

    int Cols() const noexcept;

    /**
     * 输出Vec。如果列数不为1，抛出异常。
     * @exception runtime_error 列数不为1
     */
    Vec ToVec() const;

    Mat &SwapRow(int i, int j) noexcept;
    Mat &SwapCol(int i, int j) noexcept;

    std::string ToString() const noexcept;

    void Resize(int newRows, int newCols) noexcept;

    Mat &Zero() noexcept;

    Mat &Ones() noexcept;

    double Norm2() const noexcept;

    double NormInfinity() const noexcept;

    double NormNegInfinity() const noexcept;

    double Min() const noexcept;

    void SetValue(double value) noexcept;

    /**
     * 返回矩阵是否正定。
     */
    bool PositiveDetermine() const noexcept;

    Mat Transpose() const noexcept;

    /**
     * 计算逆矩阵。
     * @exception MathError 如果是奇异矩阵，抛出异常
     */
    Mat Inverse() const;

protected:
    int rows;
    int cols;
    std::valarray<double> data;

    friend Mat operator*(double k, const Mat &mat) noexcept;
    friend std::ostream &operator<<(std::ostream &out, const Mat &mat) noexcept;
    friend Mat EachDivide(const Mat &a, const Mat &b) noexcept;
    friend bool IsZero(const Mat &mat) noexcept;
    friend bool AllIsLessThan(const Mat &v1, const Mat &v2) noexcept;
    friend void GetCofactor(const Mat &A, Mat &temp, int p, int q, int n) noexcept;
    friend void Adjoint(const Mat &A, Mat &adj) noexcept;
    friend double Det(const Mat &A, int n) noexcept;
};

Mat operator*(double k, const Mat &mat) noexcept;

std::ostream &operator<<(std::ostream &out, const Mat &mat) noexcept;

Mat EachDivide(const Mat &a, const Mat &b) noexcept;

bool IsZero(const Mat &mat) noexcept;

bool AllIsLessThan(const Mat &v1, const Mat &v2) noexcept;

int GetMaxAbsRowIndex(const Mat &A, int rowStart, int rowEnd, int col) noexcept;

/**
 * 伴随矩阵。
 */
void Adjoint(const Mat &A, Mat &adj) noexcept;

void GetCofactor(const Mat &A, Mat &temp, int p, int q, int n) noexcept;

/**
 * 计算矩阵的行列式值。
 */
double Det(const Mat &A, int n) noexcept;

class Vec : public Mat {
public:
    explicit Vec(int rows, double initValue = 0) noexcept;

    Vec(std::initializer_list<double> init) noexcept;

    Vec(std::valarray<double> data) noexcept;

    Mat &AsMat() noexcept;

    void Resize(int newRows) noexcept;

    double &operator[](std::size_t i) noexcept;

    double operator[](std::size_t i) const noexcept;

    Vec operator+(const Vec &b) const noexcept;

    // be negative
    Vec operator-() noexcept;

    Vec operator-(const Vec &b) const noexcept;

    Vec operator*(double m) const noexcept;

    Vec operator*(const Vec &b) const noexcept;

    Vec operator/(const Vec &b) const noexcept;

    bool operator<(const Vec &b) noexcept;

    friend double Dot(const Vec &a, const Vec &b) noexcept;
    friend Vec operator*(double k, const Vec &V);
};

/**
 * 向量点乘。
 */
double Dot(const Vec &a, const Vec &b) noexcept;

} // namespace tomsolver

namespace tomsolver {

Mat::Mat(int rows, int cols, double initValue) noexcept : rows(rows), cols(cols), data(initValue, rows * cols) {
    assert(rows > 0);
    assert(cols > 0);
}

Mat::Mat(std::initializer_list<std::initializer_list<double>> init) noexcept {
    rows = static_cast<int>(init.size());
    assert(rows > 0);
    cols = static_cast<int>(std::max(init, [](auto lhs, auto rhs) {
                                return lhs.size() < rhs.size();
                            }).size());
    assert(cols > 0);
    data.resize(rows * cols);

    auto i = 0;
    for (auto values : init) {
        Row(i++) = values;
    }
}

Mat::Mat(int rows, int cols, std::valarray<double> data) noexcept : rows(rows), cols(cols), data(std::move(data)) {}

std::slice_array<double> Mat::Row(int i, int offset) {
    return data[std::slice(cols * i + offset, cols - offset, 1)];
}

std::slice_array<double> Mat::Col(int j, int offset) {
    return data[std::slice(j + offset * cols, rows - offset, cols)];
}

auto Mat::Row(int i, int offset) const -> decltype(std::declval<const std::valarray<double>>()[std::slice{}]) {
    return data[std::slice(cols * i + offset, cols - offset, 1)];
}

auto Mat::Col(int j, int offset) const -> decltype(std::declval<const std::valarray<double>>()[std::slice{}]) {
    return data[std::slice(j + offset * cols, rows - offset, cols)];
}

const double &Mat::Value(int i, int j) const {
    return data[i * cols + j];
}

double &Mat::Value(int i, int j) {
    return data[i * cols + j];
}

bool Mat::operator==(double m) const noexcept {
    return std::all_of(std::begin(data), std::end(data), [m](auto val) {
        return std::abs(val - m) < Config::get().epsilon;
    });
}

bool Mat::operator==(const Mat &b) const noexcept {
    assert(rows == b.rows);
    assert(cols == b.cols);
    return std::all_of(std::begin(data), std::end(data), [iter = std::begin(b.data)](auto val) mutable {
        return std::abs(val - *iter++) < Config::get().epsilon;
    });
}

// be negative
Mat Mat::operator-() noexcept {
    return {rows, cols, -data};
}

Mat Mat::operator+(const Mat &b) const noexcept {
    assert(rows == b.rows);
    assert(cols == b.cols);
    return {rows, cols, data + b.data};
}

Mat &Mat::operator+=(const Mat &b) noexcept {
    assert(rows == b.rows);
    assert(cols == b.cols);
    data += b.data;
    return *this;
}

Mat Mat::operator-(const Mat &b) const noexcept {
    assert(rows == b.rows);
    assert(cols == b.cols);
    return {rows, cols, data - b.data};
}

Mat Mat::operator*(double m) const noexcept {
    return {rows, cols, data * m};
}

Mat Mat::operator*(const Mat &b) const noexcept {
    assert(cols == b.rows);
    Mat ans(rows, b.cols);
    for (auto i = 0; i < rows; ++i) {
        for (auto j = 0; j < b.cols; ++j) {
            ans.Value(i, j) = (Row(i) * b.Col(j)).sum();
        }
    }
    return ans;
}

int Mat::Rows() const noexcept {
    return rows;
}

int Mat::Cols() const noexcept {
    return cols;
}

Vec Mat::ToVec() const {
    assert(rows > 0);
    if (cols != 1) {
        throw std::runtime_error("Mat::ToVec fail. rows is not one");
    }
    Vec v(rows);
    v.cols = 1;
    v.data = data;
    return v;
}

Mat &Mat::SwapRow(int i, int j) noexcept {
    if (i == j) {
        return *this;
    }
    assert(i >= 0);
    assert(i < rows);
    assert(j >= 0);
    assert(j < rows);

    std::valarray temp = Row(i);
    Row(i) = Row(j);
    Row(j) = temp;

    return *this;
}

Mat &Mat::SwapCol(int i, int j) noexcept {
    if (i == j) {
        return *this;
    }
    assert(i >= 0);
    assert(i < cols);
    assert(j >= 0);
    assert(j < cols);

    std::valarray t = Col(i);
    Col(i) = Col(j);
    Col(j) = t;

    return *this;
}

std::string Mat::ToString() const noexcept {
    if (data.size() == 0) {
        return "[]";
    }

    std::stringstream ss;
    ss << "[";

    size_t i = 0;
    for (auto val : data) {
        ss << (i == 0 ? "" : " ") << tomsolver::ToString(val);
        i++;
        ss << (i % cols == 0 ? (i == data.size() ? "]" : "\n") : ", ");
    }

    return ss.str();
}

void Mat::Resize(int newRows, int newCols) noexcept {
    assert(newRows > 0 && newCols > 0);
    auto temp = std::move(data);
    data.resize(newRows * newCols);
    auto minRows = std::min<size_t>(rows, newRows);
    auto minCols = std::min<size_t>(cols, newCols);
    data[std::gslice(0, {minRows, minCols}, {static_cast<size_t>(newCols), 1})] =
        temp[std::gslice(0, {minRows, minCols}, {static_cast<size_t>(cols), 1})];
    rows = newRows;
    cols = newCols;
}

Mat &Mat::Zero() noexcept {
    data = 0;
    return *this;
}

Mat &Mat::Ones() noexcept {
    assert(rows == cols);
    Zero();
    data[std::slice(0, rows, cols + 1)] = 1;
    return *this;
}

double Mat::Norm2() const noexcept {
    return (data * data).sum();
}

double Mat::NormInfinity() const noexcept {
    return std::abs(data).max();
}

double Mat::NormNegInfinity() const noexcept {
    return std::abs(data).min();
}

double Mat::Min() const noexcept {
    return data.min();
}

void Mat::SetValue(double value) noexcept {
    data = value;
}

bool Mat::PositiveDetermine() const noexcept {
    assert(rows == cols);
    for (int i = 1; i <= rows; ++i) {
        if (Det(*this, i) <= 0) {
            return false;
        }
    }
    return true;
}

Mat Mat::Transpose() const noexcept {
    Mat ans(cols, rows);
    for (auto i = 0; i < cols; i++) {
        ans.Row(i) = Col(i);
    }
    return ans;
}

Mat Mat::Inverse() const {
    assert(rows == cols);
    int n = rows;
    double det = Det(*this, n); // Determinant, 역행렬을 시킬 행렬의 행렬식을 구함

    if (std::abs(det) <= Config::get().epsilon) // 0일때는 예외처리 (역행렬을 구할 수 없기 때문.)
    {
        throw MathError(ErrorType::ERROR_SINGULAR_MATRIX);
    }

    Mat adj(n, n); // 딸림행렬 선언

    Adjoint(*this, adj); // 딸림행렬 초기화

    return {n, n, adj.data / det};
}

Mat operator*(double k, const Mat &mat) noexcept {
    Mat ans(mat);
    ans.data *= k;
    return ans;
}

Mat EachDivide(const Mat &a, const Mat &b) noexcept {
    assert(a.rows == b.rows);
    assert(a.cols == b.cols);
    return {a.rows, b.cols, b.data / b.data};
}

bool IsZero(const Mat &mat) noexcept {
    return std::all_of(std::begin(mat.data), std::end(mat.data), [](auto val) {
        return std::abs(val) <= Config::get().epsilon;
    });
}

bool AllIsLessThan(const Mat &v1, const Mat &v2) noexcept {
    assert(v1.rows == v2.rows && v1.cols == v2.cols);
    return std::all_of(std::begin(v1.data), std::end(v1.data), [iter = std::begin(v2.data)](auto val) mutable {
        return val < *iter++;
    });
}

int GetMaxAbsRowIndex(const Mat &A, int rowStart, int rowEnd, int col) noexcept {
    std::valarray temp = std::abs(A.Col(col)[std::slice(rowStart, rowEnd - rowStart + 1, 1)]);
    return std::distance(std::begin(temp), std::find(std::begin(temp), std::end(temp), temp.max())) + rowStart;
}

void Adjoint(const Mat &A, Mat &adj) noexcept // 딸림행렬, 수반행렬
{
    if (A.rows == 1) // 예외처리
    {
        adj.Value(0, 0) = 1;
        return;
    }

    Mat cofactor(A.rows - 1, A.cols - 1);

    for (int i = 0; i < A.rows; i++) {
        for (int j = 0; j < A.cols; j++) {
            GetCofactor(A, cofactor, i, j, A.rows); // 여인수 구하기, 단 i, j값으로 되기에 temp는 항상 바뀐다.

            auto det = (Det(cofactor, A.rows - 1));

            if ((i + j) % 2 != 0) {
                det = -det; // +, -, + 형식으로 되는데, 0,0 좌표면 +, 0,1좌표면 -, 이렇게 된다.
            }

            adj.Value(j, i) = det; // n - 1 X n - 1 은, 언제나 각 여인수 행렬 은
                                   // 여인수를 따오는 행렬의 크기 - 1 이기 때문이다.
        }
    }
}

void GetCofactor(const Mat &A, Mat &cofactor, int p, int q,
                 int n) noexcept // 여인수를 구해다주는 함수!
{
    /*
         ┌───┄┄┄┄┄┄┄┄┬───┬┄┄┄┄┄┄┄┄───┐   size of region A = p * q
    0 -> │           │   │           │                  B = p * (n - 1 - q)
         ┆           ┆   ┆           ┆                  C = (n - 1 - p) * q
         ┆     A     ┆   ┆     B     ┆                  D = (n - 1 - p) * (n - 1 - q)
         ┆           ┆   ┆           ┆
         ┆           ┆   ┆           ┆    left top of region
         ├───┄┄┄┄┄┄┄┄┼───┼┄┄┄┄┄┄┄┄───┤   ╔════════╤════════════════╤══════════╗
    p ─> │           │   │           │   ║ region │ origin matrix  │ cofactor ║
         ├───┄┄┄┄┄┄┄┄┼───┼┄┄┄┄┄┄┄┄───┤   ╠════════╪════════════════╪══════════╣
         ┆           ┆   ┆           ┆   ║ A      │ (0, 0)         │ (0, 0)   ║
         ┆           ┆   ┆           ┆   ╟────────┼────────────────┼──────────╢
         ┆     C     ┆   ┆     D     ┆   ║ B      │ (0, q + 1)     │ (0, q)   ║
         ┆           ┆   ┆           ┆   ╟────────┼────────────────┼──────────╢
         │           │   │           │   ║ C      │ (p + 1, 0)     │ (p, 0)   ║
    n ─> └───┄┄┄┄┄┄┄┄┴───┴┄┄┄┄┄┄┄┄───┘   ╟────────┼────────────────┼──────────╢
          ^            ^            ^    ║ D      │ (p + 1, q + 1) │ (p, q)   ║
          0            q            n    ╚════════╧════════════════╧══════════╝
    */

    auto newIndex = [n = n - 1](int p, int q) -> size_t {
        return p * n + q;
    };
    auto index = [n = A.cols](int p, int q) -> size_t {
        return p * n + q;
    };
    auto makeValarray = [](int p, int q) {
        return std::valarray<size_t>{static_cast<size_t>(p), static_cast<size_t>(q)};
    };
    auto newStride = makeValarray(n - 1, 1);
    auto stride = makeValarray(A.cols, 1);

    std::array config = {
        std::tuple{makeValarray(p, q), newIndex(0, 0), index(0, 0)},
        std::tuple{makeValarray(p, n - 1 - q), newIndex(0, q), index(0, q + 1)},
        std::tuple{makeValarray(n - 1 - p, q), newIndex(p, 0), index(p + 1, 0)},
        std::tuple{makeValarray(n - 1 - p, n - 1 - q), newIndex(p, q), index(p + 1, q + 1)},
    };

    for (const auto &[size, newStart, start] : config) {
        if (newStart < cofactor.data.size()) {
            cofactor.data[std::gslice(newStart, size, newStride)] = A.data[std::gslice(start, size, stride)];
        }
    }
}

double Det(const Mat &A, int n) noexcept {
    if (n == 0) {
        return 0;
    }

    if (n == 1) {
        return A.Value(0, 0);
    }

    if (n == 2) // 계산 압축
    {
        return A.Value(0, 0) * A.Value(1, 1) - A.Value(1, 0) * A.Value(0, 1);
    }

    Mat cofactor(n - 1, n - 1); // n X n 행렬의 여인수를 담을 임시 행렬

    double D = 0; // D = 한 행렬의 Determinant값

    int sign = 1; // sign = +, -, +, -.... 형태로 지속되는 결과값에 영향을 주는 정수

    for (int f = 0; f < n; f++) {
        GetCofactor(A, cofactor, 0, f, n); // 0으로 고정시킨 이유는, 수학 공식 상 Determinant (행렬식)은 n개의 열 중
        // 아무거나 잡아도 결과값은 모두 일치하기 때문
        auto det = Det(cofactor, n - 1);
        auto v = A.Value(0, f);
        D += sign * v * det; // 재귀 형식으로 돌아간다. f는 n X n 중 정수 n을 향해 간다.

        sign = -sign; // +, -, +, -... 형식으로 되기 때문에 반대로 만들어준다.
    }

    return D; // 마지막엔 n X n 행렬의 Determinant를 리턴해준다.
}

Vec::Vec(int rows, double initValue) noexcept : Mat(rows, 1, initValue) {}

Vec::Vec(std::initializer_list<double> init) noexcept : Vec(std::valarray<double>{init}) {}

Vec::Vec(std::valarray<double> init) noexcept : Vec(static_cast<int>(init.size())) {
    data = std::move(init);
}

Mat &Vec::AsMat() noexcept {
    return *this;
}

void Vec::Resize(int newRows) noexcept {
    assert(newRows > 0);
    Mat::Resize(newRows, 1);
}

double &Vec::operator[](std::size_t i) noexcept {
    return data[i];
}

double Vec::operator[](std::size_t i) const noexcept {
    return data[i];
}

Vec Vec::operator+(const Vec &b) const noexcept {
    assert(rows == b.rows);
    assert(cols == 1 && b.cols == 1);
    return {data + b.data};
}

Vec Vec::operator-() noexcept {
    return {-data};
}

Vec Vec::operator-(const Vec &b) const noexcept {
    assert(rows == b.rows);
    return {data - b.data};
}

Vec Vec::operator*(double m) const noexcept {
    return {data * m};
}

Vec Vec::operator*(const Vec &b) const noexcept {
    assert(rows == b.rows);
    return {data * b.data};
}

Vec Vec::operator/(const Vec &b) const noexcept {
    assert(rows == b.rows);
    return {data / b.data};
}

bool Vec::operator<(const Vec &b) noexcept {
    assert(rows == b.rows);
    return std::all_of(std::begin(data), std::end(data), [iter = std::begin(b.data)](auto val) mutable {
        return val < *iter++;
    });
}

Vec operator*(double k, const Vec &v) {
    return {v.data * k};
}

double Dot(const Vec &a, const Vec &b) noexcept {
    assert(a.rows == b.rows);
    return (a.data * b.data).sum();
}

std::ostream &operator<<(std::ostream &out, const Mat &mat) noexcept {
    return out << mat.ToString();
}

} // namespace tomsolver

namespace tomsolver {

/**
 * 求解线性方程组Ax = b。传入矩阵A，向量b，返回向量x。
 * @exception MathError 奇异矩阵
 * @exception MathError 矛盾方程组
 * @exception MathError 不定方程（设置Config::get().allowIndeterminateEquation=true可以允许不定方程组返回一组特解）
 *
 */
Vec SolveLinear(Mat A, Vec b);

} // namespace tomsolver

namespace tomsolver {

Vec SolveLinear(Mat A, Vec b) {
    int rows = A.Rows(); // 行数
    int cols = rows;     // 列数=未知数个数

    int RankA = rows, RankAb = rows; // 初始值

    assert(rows == b.Rows()); // A行数不等于b行数

    Vec ret(rows);

    if (rows > 0) {
        cols = A.Cols();
    }
    if (cols != rows) // 不是方阵
    {
        if (rows > cols) {
            // 过定义方程组
            throw MathError(ErrorType::ERROR_OVER_DETERMINED_EQUATIONS);
        } else {
            // 不定方程组
            ret.Resize(cols);
        }
    }

    std::vector<int> TrueRowNumber(cols);

    // 列主元消元法
    for (auto y = 0, x = 0; y < rows && x < cols; y++, x++) {
        // if (A[i].size() != rows)

        // 从当前行(y)到最后一行(rows-1)中，找出x列最大的一行与y行交换
        int maxAbsRowIndex = GetMaxAbsRowIndex(A, y, rows - 1, x);
        A.SwapRow(y, maxAbsRowIndex);
        b.SwapRow(y, maxAbsRowIndex);

        while (std::abs(A.Value(y, x)) < Config::get().epsilon) // 如果当前值为0  x一直递增到非0
        {
            x++;
            if (x == cols) {
                break;
            }

            // 交换本行与最大行
            maxAbsRowIndex = GetMaxAbsRowIndex(A, y, rows - 1, x);
            A.SwapRow(y, maxAbsRowIndex);
            b.SwapRow(y, maxAbsRowIndex);
        }

        if (x != cols && x > y) {
            TrueRowNumber[y] = x; // 补齐方程时 当前行应换到x行
        }

        if (x == cols) // 本行全为0
        {
            RankA = y;
            if (std::abs(b[y]) < Config::get().epsilon) {
                RankAb = y;
            }

            if (RankA != RankAb) {
                // 奇异，且系数矩阵及增广矩阵秩不相等->无解
                throw MathError(ErrorType::ERROR_SINGULAR_MATRIX);
            } else {
                // 跳出for，得到特解
                break;
            }
        }

        // 主对角线化为1
        auto ratio = A.Value(y, x);
        // y行第j个->第cols个
        std::valarray rowY = std::as_const(A).Row(y, x) / ratio;
        A.Row(y, x) = rowY;
        b[y] /= ratio;

        // 每行化为0
        for (auto row = y + 1; row < rows; row++) // 下1行->最后1行
        {
            if (auto ratio = A.Value(row, x); std::abs(A.Value(row, x)) >= Config::get().epsilon) {
                A.Row(row, x) -= rowY * ratio;
                b[row] -= b[y] * ratio;
            }
        }
    }

    bool bIndeterminateEquation = false; // 设置此变量是因为后面rows将=cols，标记以判断是否为不定方程组

    // 若为不定方程组，空缺行全填0继续运算
    if (rows != cols) {
        A.Resize(cols, cols);
        b.Resize(cols);
        rows = cols;
        bIndeterminateEquation = true;

        // 调整顺序
        for (int i = rows - 1; i >= 0; i--) {
            if (TrueRowNumber[i] != 0) {
                A.SwapRow(i, TrueRowNumber[i]);
                b.SwapRow(i, TrueRowNumber[i]);
            }
        }
    }

    // 后置换得到x
    for (int i = rows - 1; i >= 0; i--) // 最后1行->第1行
    {
        ret[i] = b[i] - (std::as_const(A).Row(i, i + 1) * std::as_const(ret).Col(0, i + 1)).sum();
    }

    if (RankA < cols && RankA == RankAb) {
        if (bIndeterminateEquation) {
            if (!Config::get().allowIndeterminateEquation) {
                throw MathError(ErrorType::ERROR_INDETERMINATE_EQUATION,
                                "A = " + A.ToString() + "\nb = " + b.ToString());
            }
        } else {
            throw MathError(ErrorType::ERROR_INFINITY_SOLUTIONS);
        }
    }

    return ret;
}

} // namespace tomsolver

namespace tomsolver {

/**
 * 变量表。
 * 内部保存了多个变量名及其数值的对应关系。
 */
class VarsTable {
public:
    /**
     * 新建变量表。
     * @param vars 变量数组
     * @param initValue 初值
     */
    VarsTable(const std::vector<std::string> &vars, double initValue);

    /**
     * 新建变量表。
     * @param vars 变量数组
     * @param initValue 初值
     */
    explicit VarsTable(std::initializer_list<std::pair<std::string, double>> initList);

    /**
     * 新建变量表。
     * @param vars 变量数组
     * @param initValue 初值
     */
    explicit VarsTable(const std::map<std::string, double> &table) noexcept;

    /**
     * 变量数量。
     */
    int VarNums() const noexcept;

    /**
     * 返回std::vector容器包装的变量名数组。
     */
    const std::vector<std::string> &Vars() const noexcept;

    /**
     * 返回所有变量名对应的值的数值向量。
     */
    const Vec &Values() const noexcept;

    /**
     * 设置数值向量。
     */
    void SetValues(const Vec &v) noexcept;

    /**
     * 返回是否有指定的变量。
     */
    bool Has(const std::string &varname) const noexcept;

    std::map<std::string, double>::const_iterator begin() const noexcept;

    std::map<std::string, double>::const_iterator end() const noexcept;

    std::map<std::string, double>::const_iterator cbegin() const noexcept;

    std::map<std::string, double>::const_iterator cend() const noexcept;

    bool operator==(const VarsTable &rhs) const noexcept;

    /**
     * 根据变量名获取数值。
     * @exception out_of_range 如果没有这个变量，抛出异常
     */
    double operator[](const std::string &varname) const;

private:
    std::vector<std::string> vars;
    Vec values;
    std::map<std::string, double> table;
};

std::ostream &operator<<(std::ostream &out, const VarsTable &table) noexcept;

} // namespace tomsolver

namespace tomsolver {

VarsTable::VarsTable(const std::vector<std::string> &vars, double initValue)
    : vars(vars), values(static_cast<int>(vars.size()), initValue) {
    for (auto &var : vars) {
        table.try_emplace(var, initValue);
    }
    assert(vars.size() == table.size() && "vars is not unique");
}

VarsTable::VarsTable(std::initializer_list<std::pair<std::string, double>> initList)
    : VarsTable({initList.begin(), initList.end()}) {
    assert(vars.size() == table.size() && "vars is not unique");
}

VarsTable::VarsTable(const std::map<std::string, double> &table) noexcept
    : vars(table.size()), values(static_cast<int>(table.size())), table(table) {
    int i = 0;
    for (auto &[var, val] : table) {
        vars[i] = var;
        values[i] = val;
        ++i;
    }
}

int VarsTable::VarNums() const noexcept {
    return static_cast<int>(table.size());
}

const std::vector<std::string> &VarsTable::Vars() const noexcept {
    return vars;
}

const Vec &VarsTable::Values() const noexcept {
    return values;
}

void VarsTable::SetValues(const Vec &v) noexcept {
    assert(v.Rows() == values.Rows());
    values = v;
    for (int i = 0; i < values.Rows(); ++i) {
        table[vars[i]] = v[i];
    }
}

bool VarsTable::Has(const std::string &varname) const noexcept {
    return table.find(varname) != table.end();
}

std::map<std::string, double>::const_iterator VarsTable::begin() const noexcept {
    return table.begin();
}

std::map<std::string, double>::const_iterator VarsTable::end() const noexcept {
    return table.end();
}

std::map<std::string, double>::const_iterator VarsTable::cbegin() const noexcept {
    return table.cbegin();
}

std::map<std::string, double>::const_iterator VarsTable::cend() const noexcept {
    return table.cend();
}

bool VarsTable::operator==(const VarsTable &rhs) const noexcept {
    return values.Rows() == rhs.values.Rows() &&
           std::equal(table.begin(), table.end(), rhs.table.begin(), [](const auto &lhs, const auto &rhs) {
               auto &[lVar, lVal] = lhs;
               auto &[rVar, rVal] = rhs;
               return lVar == rVar && std::abs(lVal - rVal) <= Config::get().epsilon;
           });
}

double VarsTable::operator[](const std::string &varname) const {
    auto it = table.find(varname);
    if (it == table.end()) {
        throw std::out_of_range("no such variable: " + varname);
    }
    return it->second;
}

std::ostream &operator<<(std::ostream &out, const VarsTable &table) noexcept {
    for (auto &[var, val] : table) {
        out << var << " = " << tomsolver::ToString(val) << std::endl;
    }
    return out;
}

} // namespace tomsolver

namespace tomsolver {

enum class NodeType { NUMBER, OPERATOR, VARIABLE };

// 前置声明
namespace internal {
struct NodeImpl;
}
class SymMat;

/**
 * 表达式节点。
 */
using Node = std::unique_ptr<internal::NodeImpl>;

namespace internal {

/**
 * 单个节点的实现。通常应该以std::unique_ptr包裹。
 */
struct NodeImpl {

    NodeImpl(NodeType type, MathOperator op, double value, std::string varname) noexcept
        : type(type), op(op), value(value), varname(varname), parent(nullptr) {}

    NodeImpl(const NodeImpl &rhs) noexcept;
    NodeImpl &operator=(const NodeImpl &rhs) noexcept;

    NodeImpl(NodeImpl &&rhs) noexcept;
    NodeImpl &operator=(NodeImpl &&rhs) noexcept;

    ~NodeImpl();

    bool Equal(const Node &rhs) const noexcept;

    /**
     * 把整个节点以中序遍历的顺序输出为字符串。
     * 例如：
     *      Node n = (Var("a") + Num(1)) * Var("b");
     *   则
     *      n->ToString() == "(a+1.000000)*b"
     */
    std::string ToString() const noexcept;

    /**
     * 计算出整个表达式的数值。不改变自身。
     * @exception runtime_error 如果有变量存在，则无法计算
     * @exception MathError 出现浮点数无效值(inf, -inf, nan)
     */
    double Vpa() const;

    /**
     * 计算出整个表达式的数值。不改变自身。
     * @exception runtime_error 如果有变量存在，则无法计算
     * @exception MathError 出现浮点数无效值(inf, -inf, nan)
     */
    NodeImpl &Calc();

    /**
     * 返回表达式内出现的所有变量名。
     */
    std::set<std::string> GetAllVarNames() const noexcept;

    /**
     * 检查整个节点数的parent指针是否正确。
     */
    void CheckParent() const noexcept;

private:
    NodeType type = NodeType::NUMBER;
    MathOperator op = MathOperator::MATH_NULL;
    double value;
    std::string varname;
    NodeImpl *parent = nullptr;
    Node left, right;
    NodeImpl() = default;

    /**
     * 本节点如果是OPERATOR，检查操作数数量和left, right指针是否匹配。
     */
    void CheckOperatorNum() const noexcept;

    /**
     * 节点转string。仅限本节点，不含子节点。
     */
    std::string NodeToStr() const noexcept;

    void ToStringRecursively(std::stringstream &output) const noexcept;

    void ToStringNonRecursively(std::stringstream &output) const noexcept;

    /**
     * 计算表达式数值。递归实现。
     * @exception runtime_error 如果有变量存在，则无法计算
     * @exception MathError 不符合定义域, 除0等情况。
     */
    double VpaRecursively() const;

    /**
     * 计算表达式数值。非递归实现。
     * 性能弱于递归实现。但不会导致栈溢出。
     * 根据benchmark，生成一组含4000个随机四则运算节点的表达式，生成1000次，Release下测试耗时3000ms。递归实现耗时2500ms。
     * 粗略计算，即 1333 ops/ms。
     * @exception runtime_error 如果有变量存在，则无法计算
     * @exception MathError 不符合定义域, 除0等情况。
     */
    double VpaNonRecursively() const;

    /**
     * 释放整个节点树，除了自己。
     * 实际是二叉树的非递归后序遍历。
     */
    void Release() noexcept;

    friend Node Operator(MathOperator op, Node left, Node right) noexcept;

    friend Node CloneRecursively(const Node &rhs) noexcept;
    friend Node CloneNonRecursively(const Node &rhs) noexcept;

    friend void CopyOrMoveTo(NodeImpl *parent, Node &child, Node &&n1) noexcept;
    friend void CopyOrMoveTo(NodeImpl *parent, Node &child, const Node &n1) noexcept;

    friend std::ostream &operator<<(std::ostream &out, const Node &n) noexcept;

    template <typename T>
    friend Node UnaryOperator(MathOperator op, T &&n) noexcept;

    template <typename T1, typename T2>
    friend Node BinaryOperator(MathOperator op, T1 &&n1, T2 &&n2) noexcept;

    friend class tomsolver::SymMat;
    friend class SimplifyFunctions;
    friend class DiffFunctions;
    friend class SubsFunctions;
    friend class ParseFunctions;
};

Node CloneRecursively(const Node &rhs) noexcept;

Node CloneNonRecursively(const Node &rhs) noexcept;

/**
 * 对于一个节点n和另一个节点n1，把n1移动到作为n的子节点。
 * 用法：CopyOrMoveTo(n->parent, n->left, std::forward<T>(n1));
 */
void CopyOrMoveTo(NodeImpl *parent, Node &child, Node &&n1) noexcept;

/**
 * 对于一个节点n和另一个节点n1，把n1整个拷贝一份，把拷贝的副本设为n的子节点。
 * 用法：CopyOrMoveTo(n->parent, n->left, std::forward<T>(n1));
 */
void CopyOrMoveTo(NodeImpl *parent, Node &child, const Node &n1) noexcept;

/**
 * 重载std::ostream的<<操作符以输出一个Node节点。
 */
std::ostream &operator<<(std::ostream &out, const Node &n) noexcept;

template <typename T>
Node UnaryOperator(MathOperator op, T &&n) noexcept {
    auto ret = std::make_unique<NodeImpl>(NodeType::OPERATOR, op, 0, "");
    CopyOrMoveTo(ret.get(), ret->left, std::forward<T>(n));
    return ret;
}

template <typename T1, typename T2>
Node BinaryOperator(MathOperator op, T1 &&n1, T2 &&n2) noexcept {
    auto ret = std::make_unique<NodeImpl>(NodeType::OPERATOR, op, 0, "");
    CopyOrMoveTo(ret.get(), ret->left, std::forward<T1>(n1));
    CopyOrMoveTo(ret.get(), ret->right, std::forward<T2>(n2));
    return ret;
}

/**
 * 新建一个运算符节点。
 */
Node Operator(MathOperator op, Node left = nullptr, Node right = nullptr) noexcept;

} // namespace internal

Node Clone(const Node &rhs) noexcept;

/**
 * 对节点进行移动。等同于std::move。
 */
Node Move(Node &rhs) noexcept;

/**
 * 新建一个数值节点。
 */
Node Num(double num) noexcept;

/**
 * 新建一个函数节点。
 */
Node Op(MathOperator op);

/**
 * 返回变量名是否有效。（只支持英文数字或者下划线，第一个字符必须是英文或者下划线）
 */
bool VarNameIsLegal(std::string_view varname) noexcept;

/**
 * 新建一个变量节点。
 * @exception runtime_error 名字不合法
 */
Node Var(std::string_view varname);

template <typename... T>
using SfinaeNode = std::enable_if_t<std::conjunction_v<std::is_same<std::decay_t<T>, Node>...>, Node>;

template <typename T1, typename T2>
SfinaeNode<T1, T2> operator+(T1 &&n1, T2 &&n2) noexcept {
    return internal::BinaryOperator(MathOperator::MATH_ADD, std::forward<T1>(n1), std::forward<T2>(n2));
}

template <typename T>
SfinaeNode<T> &operator+=(Node &n1, T &&n2) noexcept {
    n1 = internal::BinaryOperator(MathOperator::MATH_ADD, std::move(n1), std::forward<T>(n2));
    return n1;
}

template <typename T1, typename T2>
SfinaeNode<T1, T2> operator-(T1 &&n1, T2 &&n2) noexcept {
    return internal::BinaryOperator(MathOperator::MATH_SUB, std::forward<T1>(n1), std::forward<T2>(n2));
}

template <typename T>
SfinaeNode<T> operator-(T &&n1) noexcept {
    return internal::UnaryOperator(MathOperator::MATH_NEGATIVE, std::forward<T>(n1));
}

template <typename T>
SfinaeNode<T> operator+(T &&n1) noexcept {
    return internal::UnaryOperator(MathOperator::MATH_POSITIVE, std::forward<T>(n1));
}

template <typename T>
SfinaeNode<T> &operator-=(Node &n1, T &&n2) noexcept {
    n1 = internal::BinaryOperator(MathOperator::MATH_SUB, std::move(n1), std::forward<T>(n2));
    return n1;
}

template <typename T1, typename T2>
SfinaeNode<T1, T2> operator*(T1 &&n1, T2 &&n2) noexcept {
    return internal::BinaryOperator(MathOperator::MATH_MULTIPLY, std::forward<T1>(n1), std::forward<T2>(n2));
}

template <typename T>
SfinaeNode<T> &operator*=(Node &n1, T &&n2) noexcept {
    n1 = internal::BinaryOperator(MathOperator::MATH_MULTIPLY, std::move(n1), std::forward<T>(n2));
    return n1;
}

template <typename T1, typename T2>
SfinaeNode<T1, T2> operator/(T1 &&n1, T2 &&n2) noexcept {
    return internal::BinaryOperator(MathOperator::MATH_DIVIDE, std::forward<T1>(n1), std::forward<T2>(n2));
}

template <typename T>
SfinaeNode<T> &operator/=(Node &n1, T &&n2) noexcept {
    n1 = internal::BinaryOperator(MathOperator::MATH_DIVIDE, std::move(n1), std::forward<T>(n2));
    return n1;
}

template <typename T1, typename T2>
SfinaeNode<T1, T2> operator^(T1 &&n1, T2 &&n2) noexcept {
    return internal::BinaryOperator(MathOperator::MATH_POWER, std::forward<T1>(n1), std::forward<T2>(n2));
}

template <typename T>
SfinaeNode<T> &operator^=(Node &n1, T &&n2) noexcept {
    n1 = internal::BinaryOperator(MathOperator::MATH_POWER, std::move(n1), std::forward<T>(n2));
    return n1;
}

} // namespace tomsolver

namespace tomsolver {

namespace internal {

NodeImpl::NodeImpl(const NodeImpl &rhs) noexcept {
    *this = rhs;
}

NodeImpl &NodeImpl::operator=(const NodeImpl &rhs) noexcept {
    type = rhs.type;
    op = rhs.op;
    value = rhs.value;
    varname = rhs.varname;
    parent = rhs.parent;
    if (rhs.left) {
        left = Clone(rhs.left);
        left->parent = this;
    } else {
        left = nullptr;
    }
    if (rhs.right) {
        right = Clone(rhs.right);
        right->parent = this;
    } else {
        right = nullptr;
    }
    return *this;
}

NodeImpl::NodeImpl(NodeImpl &&rhs) noexcept {
    *this = std::move(rhs);
}

NodeImpl &NodeImpl::operator=(NodeImpl &&rhs) noexcept {
    type = std::exchange(rhs.type, {});
    op = std::exchange(rhs.op, {});
    value = std::exchange(rhs.value, {});
    varname = std::exchange(rhs.varname, {});
    parent = std::exchange(rhs.parent, {});
    left = std::exchange(rhs.left, {});
    if (left) {
        left->parent = this;
    }
    right = std::exchange(rhs.right, {});
    if (right) {
        right->parent = this;
    }

    return *this;
}

NodeImpl::~NodeImpl() {
    Release();
}

// 前序遍历。非递归实现。
bool NodeImpl::Equal(const Node &rhs) const noexcept {
    if (this == rhs.get()) {
        return true;
    }

    std::stack<std::tuple<const NodeImpl &, const NodeImpl &>> stk;

    auto tie = [](const NodeImpl &node) {
        return std::tie(node.type, node.op, node.value, node.varname);
    };

    auto IsSame = [&tie](const NodeImpl &lhs, const NodeImpl &rhs) {
        return tie(lhs) == tie(rhs);
    };

    auto CheckChildren = [&stk](const Node &lhs, const Node &rhs) {
        // ╔═════╦═════╦════════╦═════════╗
        // ║ lhs ║ rhs ║ return ║ emplace ║
        // ╠═════╬═════╬════════╬═════════╣
        // ║ T   ║ T   ║ T      ║ T       ║
        // ╟─────╫─────╫────────╫─────────╢
        // ║ T   ║ F   ║ F      ║ F       ║
        // ╟─────╫─────╫────────╫─────────╢
        // ║ F   ║ T   ║ F      ║ F       ║
        // ╟─────╫─────╫────────╫─────────╢
        // ║ F   ║ F   ║ T      ║ F       ║
        // ╚═════╩═════╩════════╩═════════╝
        if (!lhs ^ !rhs) {
            return false;
        }

        if (lhs && rhs) {
            stk.emplace(*lhs, *rhs);
        }

        return true;
    };

    auto CheckNode = [&IsSame, &CheckChildren](const NodeImpl &lhs, const NodeImpl &rhs) {
        return IsSame(lhs, rhs) && CheckChildren(lhs.left, rhs.left) && CheckChildren(lhs.right, rhs.right);
    };

    if (!CheckNode(*this, *rhs)) {
        return false;
    }

    while (!stk.empty()) {
        const auto &[lhs, rhs] = stk.top();
        stk.pop();

        // 检查
        if (!CheckNode(lhs, rhs)) {
            return false;
        }
    }

    return true;
}

std::string NodeImpl::ToString() const noexcept {
    std::stringstream ss;
    ToStringNonRecursively(ss);
    return ss.str();
}

double NodeImpl::Vpa() const {
    return VpaNonRecursively();
}

NodeImpl &NodeImpl::Calc() {
    auto d = Vpa();
    *this = {};
    value = d;

    return *this;
}

// 前序遍历。非递归实现。
void NodeImpl::CheckParent() const noexcept {
    std::stack<std::tuple<const NodeImpl &>> stk;

    auto EmplaceNode = [&stk](const Node &node) {
        if (node) {
            stk.emplace(*node);
        }
    };
    auto TryEmplaceChildren = [&EmplaceNode](const NodeImpl &node) {
        node.CheckOperatorNum();
        EmplaceNode(node.left);
        EmplaceNode(node.right);
    };

    TryEmplaceChildren(*this);

    while (!stk.empty()) {
        const auto &[f] = stk.top();
        stk.pop();

#ifndef NDEBUG
        // 检查
        assert(f.parent);
        bool isLeftChild = f.parent->left.get() == &f;
        bool isRightChild = f.parent->right.get() == &f;
        assert(isLeftChild || isRightChild);
#endif

        TryEmplaceChildren(f);
    }
}

void NodeImpl::CheckOperatorNum() const noexcept {
    if (type != NodeType::OPERATOR) {
        return;
    }

    switch (GetOperatorNum(op)) {
    case 1:
        assert(!right);
        break;
    case 2:
        assert(right);
        break;
    default:
        assert(0);
        break;
    }

    assert(left);
}

std::string NodeImpl::NodeToStr() const noexcept {
    switch (type) {
    case NodeType::NUMBER:
        return tomsolver::ToString(value);
    case NodeType::VARIABLE:
        return varname;
    case NodeType::OPERATOR:
        return MathOperatorToStr(op);
    }
    assert(0 && "unexpected NodeType. maybe this is a bug.");
    return "";
}

// 中序遍历。递归实现。
void NodeImpl::ToStringRecursively(std::stringstream &output) const noexcept {
    switch (type) {
    case NodeType::NUMBER:
        // 如果当前节点是数值且小于0，且前面是-运算符，那么加括号
        if (value < 0 && parent && parent->right.get() == this && parent->op == MathOperator::MATH_SUB) {
            output << "(" << NodeToStr() << ")";
        } else {
            output << NodeToStr();
        }
        return;
    case NodeType::VARIABLE:
        output << NodeToStr();
        return;
    case NodeType::OPERATOR:
        // pass
        break;
    }

    auto hasParenthesis = false;
    auto operatorNum = GetOperatorNum(op);
    if (operatorNum == 1) // 一元运算符：函数和取负
    {
        if (op == MathOperator::MATH_POSITIVE || op == MathOperator::MATH_NEGATIVE) {
            output << "(" << NodeToStr();
        } else {
            output << NodeToStr() << "(";
        }
        hasParenthesis = true;
    } else {
        // 非一元运算符才输出，即一元运算符的输出顺序已改变
        if (type == NodeType::OPERATOR && parent) { // 本级为运算符
            if ((GetOperatorNum(parent->op) == 2 && // 父运算符存在，为二元，
                 (Rank(parent->op) > Rank(op)       // 父级优先级高于本级->加括号

                  || ( // 两级优先级相等
                         Rank(parent->op) == Rank(op) &&
                         (
                             // 本级为父级的右子树 且父级不满足结合律->加括号
                             (InAssociativeLaws(parent->op) == false && this == parent->right.get()) ||
                             // 两级都是右结合
                             (InAssociativeLaws(parent->op) == false && IsLeft2Right(op) == false)))))

                //||

                ////父运算符存在，为除号，且本级为分子，则添加括号
                //(now->parent->eOperator == MATH_DIVIDE && now == now->parent->right)
            ) {
                output << "(";
                hasParenthesis = true;
            }
        }
    }

    if (left) // 左遍历
    {
        left->ToStringRecursively(output);
    }

    if (operatorNum != 1) // 非一元运算符才输出，即一元运算符的输出顺序已改变
    {
        output << NodeToStr();
    }

    if (right) // 右遍历
    {
        right->ToStringRecursively(output);
    }

    // 回到本级时补齐右括号，包住前面的东西
    if (hasParenthesis) {
        output << ")";
    }
}

// 中序遍历。非递归实现。
void NodeImpl::ToStringNonRecursively(std::stringstream &output) const noexcept {
    std::stack<std::tuple<const NodeImpl &>> stk;

    NodeImpl rightParenthesis(NodeType::OPERATOR, MathOperator::MATH_RIGHT_PARENTHESIS, 0, "");

    auto AddLeftLine = [&stk, &output, &rightParenthesis](const NodeImpl *cur) {
        while (cur) {
            if (cur->type != NodeType::OPERATOR) {
                stk.emplace(*cur);
                cur = cur->left.get();
                continue;
            }

            // 一元运算符的特殊处理：
            //      例如sin: 直接输出 "sin(" ，并且把一个右括号入栈。让退栈时这个右括号能包裹住现在的子树。
            //      如果是+/-: 直接输出 "+"/"-"，如果+/-的操作数是operator，那么处理方式和sin这类一样；
            //                                  如果+/-的操作数是number/variable，那么不加括号。
            if (GetOperatorNum(cur->op) == 1) {
                if ((cur->op == MathOperator::MATH_POSITIVE || cur->op == MathOperator::MATH_NEGATIVE) &&
                    (cur->left->type != NodeType::OPERATOR)) {
                    output << cur->NodeToStr();
                    cur = cur->left.get();
                    continue;
                }
                output << cur->NodeToStr() << "(";

                // not push this op

                // push ')'
                stk.emplace(rightParenthesis);

                cur = cur->left.get();
                continue;
            }

            // 二元运算符的特殊处理：
            if (cur->parent) {
                if ((GetOperatorNum(cur->parent->op) == 2 && // 父运算符存在，为二元，
                     (Rank(cur->parent->op) > Rank(cur->op)  // 父级优先级高于本级->加括号

                      || ( // 两级优先级相等
                             Rank(cur->parent->op) == Rank(cur->op) &&
                             (
                                 // 本级为父级的右子树 且父级不满足结合律->加括号
                                 (InAssociativeLaws(cur->parent->op) == false && cur == cur->parent->right.get()) ||
                                 // 两级都是右结合
                                 (InAssociativeLaws(cur->parent->op) == false && IsLeft2Right(cur->op) == false)))))

                    //||

                    ////父运算符存在，为除号，且本级为分子，则添加括号
                    //(now->parent->eOperator == MATH_DIVIDE && now == now->parent->right)
                ) {
                    output << "(";

                    // push ')'
                    stk.emplace(rightParenthesis);

                    stk.emplace(*cur);
                    cur = cur->left.get();
                    continue;
                }
            }

            stk.emplace(*cur);
            cur = cur->left.get();
        }
    };

    AddLeftLine(this);

    while (!stk.empty()) {
        const auto &[cur] = stk.top();
        stk.pop();

        // output

        // 负数的特殊处理
        // 如果当前节点是数值且小于0，且前面是-运算符，那么加括号
        if (cur.type == NodeType::NUMBER && cur.value < 0 && cur.parent && cur.parent->right.get() == &cur &&
            cur.parent->op == MathOperator::MATH_SUB) {
            output << "(" << cur.NodeToStr() << ")";
        } else {
            output << cur.NodeToStr();
        }

        if (cur.right) {
            AddLeftLine(cur.right.get());
            continue;
        }
    }
}

// 后序遍历。递归实现。
double NodeImpl::VpaRecursively() const {

    auto vpa = [](const Node &node) {
        return node ? node->Vpa() : 0;
    };

    switch (type) {
    case NodeType::NUMBER:
        return value;

    case NodeType::VARIABLE:
        throw std::runtime_error("has variable. can not calculate to be a number");

    case NodeType::OPERATOR:
        assert((GetOperatorNum(op) == 1 && left && right == nullptr) || (GetOperatorNum(op) == 2 && left && right));
        return tomsolver::Calc(op, vpa(left), vpa(right));
    }

    throw std::runtime_error("unsupported node type");
}

// 后序遍历。非递归实现。
double NodeImpl::VpaNonRecursively() const {

    std::stack<std::tuple<const NodeImpl &>> stk;
    std::forward_list<std::tuple<const NodeImpl &>> revertedPostOrder;

    // ==== Part I ====

    // 借助一个栈，得到反向的后序遍历序列，结果保存在revertedPostOrder
    stk.emplace(*this);

    while (!stk.empty()) {
        const auto &[node] = stk.top();
        stk.pop();

        if (node.left) {
            stk.emplace(*node.left);
        }

        if (node.right) {
            stk.emplace(*node.right);
        }

        revertedPostOrder.emplace_front(node);
    }

    // ==== Part II ====
    // revertedPostOrder的反向序列是一组逆波兰表达式，根据这组逆波兰表达式可以计算出表达式的值
    // calcStk是用来计算值的临时栈，计算完成后calcStk的size应该为1
    std::stack<double> calcStk;
    // for (auto it = revertedPostOrder.rbegin(); it != revertedPostOrder.rend(); ++it) {
    for (const auto &[node] : revertedPostOrder) {
        switch (node.type) {
        case NodeType::NUMBER:
            calcStk.emplace(node.value);
            break;

        case NodeType::OPERATOR: {
            auto r = std::numeric_limits<double>::quiet_NaN();

            switch (GetOperatorNum(node.op)) {
            case 1:
                break;
            case 2:
                r = calcStk.top();
                calcStk.pop();
                break;
            default:
                assert(0 && "[VpaNonRecursively] unsupported operator num");
                break;
            }

            auto &l = calcStk.top();
            l = tomsolver::Calc(node.op, l, r);
            break;
        }

        default:
            throw std::runtime_error("wrong");
            break;
        }
    }

    assert(calcStk.size() == 1);

    return calcStk.top();
}

// 后序遍历。因为要在左右儿子都没有的情况下删除节点。
void NodeImpl::Release() noexcept {
    std::stack<Node> stk;

    auto emplaceNode = [&stk](Node node) {
        if (node) {
            stk.emplace(std::move(node));
        }
    };

    auto emplaceChildren = [&emplaceNode](NodeImpl &node) {
        emplaceNode(std::move(node.left));
        emplaceNode(std::move(node.right));
    };

    emplaceChildren(*this);

    while (!stk.empty()) {
        auto node = std::move(stk.top());
        stk.pop();

        emplaceChildren(*node);

        assert(!node->left && !node->right);

        // 这里如果把node填入vector，最后翻转。得到的序列就是后序遍历。

        // 这里node会被自动释放。
    }
}

Node CloneRecursively(const Node &src) noexcept {
    auto ret = std::make_unique<NodeImpl>(src->type, src->op, src->value, src->varname);
    auto Copy = [ret = ret.get()](Node &tgt, const Node &src) {
        if (src) {
            tgt = Clone(src);
            tgt->parent = ret;
        }
    };

    Copy(ret->left, src->left);
    Copy(ret->right, src->right);

    return ret;
}

// 前序遍历。非递归实现。
Node CloneNonRecursively(const Node &src) noexcept {
    std::stack<std::tuple<const NodeImpl &, NodeImpl &, Node &>> stk;

    auto MakeNode = [](const NodeImpl &src, NodeImpl *parent = nullptr) {
        auto node = std::make_unique<NodeImpl>(src.type, src.op, src.value, src.varname);
        node->parent = parent;
        return node;
    };

    auto EmplaceNode = [&stk](const Node &src, NodeImpl &parent, Node &tgt) {
        if (src) {
            stk.emplace(*src, parent, tgt);
        }
    };

    auto EmplaceChildren = [&EmplaceNode](const NodeImpl &src, Node &tgt) {
        EmplaceNode(src.left, *tgt, tgt->left);
        EmplaceNode(src.right, *tgt, tgt->right);
    };

    auto ret = MakeNode(*src);
    EmplaceChildren(*src, ret);

    while (!stk.empty()) {
        const auto &[src, parent, tgt] = stk.top();
        stk.pop();

        tgt = MakeNode(src, &parent);
        EmplaceChildren(src, tgt);
    }

    return ret;
}

void CopyOrMoveTo(NodeImpl *parent, Node &child, Node &&n1) noexcept {
    n1->parent = parent;
    child = std::move(n1);
}

void CopyOrMoveTo(NodeImpl *parent, Node &child, const Node &n1) noexcept {
    auto n1Clone = std::make_unique<NodeImpl>(*n1);
    n1Clone->parent = parent;
    child = std::move(n1Clone);
}

std::ostream &operator<<(std::ostream &out, const Node &n) noexcept {
    out << n->ToString();
    return out;
}

Node Operator(MathOperator op, Node left, Node right) noexcept {
    auto ret = std::make_unique<internal::NodeImpl>(NodeType::OPERATOR, op, 0, "");

    auto SetChild = [ret = ret.get()](Node &tgt, Node src) {
        if (src) {
            src->parent = ret;
            tgt = std::move(src);
        }
    };

    SetChild(ret->left, std::move(left));
    SetChild(ret->right, std::move(right));

    return ret;
}

// 前序遍历。非递归实现。
std::set<std::string> NodeImpl::GetAllVarNames() const noexcept {
    std::set<std::string> ret;

    std::stack<std::tuple<const NodeImpl &>> stk;

    auto EmplaceNode = [&stk](const Node &node) {
        if (node) {
            stk.emplace(*node);
        }
    };

    auto EmplaceChild = [&ret, &EmplaceNode](const NodeImpl &node) {
        if (node.type == NodeType::VARIABLE) {
            ret.emplace(node.varname);
        }
        EmplaceNode(node.left);
        EmplaceNode(node.right);
    };

    EmplaceChild(*this);

    while (!stk.empty()) {
        const auto &[node] = stk.top();
        stk.pop();
        EmplaceChild(node);
    }

    return ret;
}

} // namespace internal

Node Clone(const Node &rhs) noexcept {
    return internal::CloneNonRecursively(rhs);
}

Node Move(Node &rhs) noexcept {
    return std::move(rhs);
}

Node Num(double num) noexcept {
    return std::make_unique<internal::NodeImpl>(NodeType::NUMBER, MathOperator::MATH_NULL, num, "");
}

Node Op(MathOperator op) {
    if (op == MathOperator::MATH_NULL) {
        throw std::runtime_error("Illegal MathOperator: MATH_NULL");
    }
    return std::make_unique<internal::NodeImpl>(NodeType::OPERATOR, op, 0, "");
}

bool VarNameIsLegal(std::string_view varname) noexcept {
    return std::regex_match(varname.begin(), varname.end(), std::regex{R"((?=\w)\D\w*)"});
}

Node Var(std::string_view varname) {
    auto name = std::string{varname};
    if (!VarNameIsLegal(varname)) {
        throw std::runtime_error("Illegal varname: " + name);
    }
    return std::make_unique<internal::NodeImpl>(NodeType::VARIABLE, MathOperator::MATH_NULL, 0, std::move(name));
}

} // namespace tomsolver

namespace tomsolver {

template <typename T>
Node sin(T &&n) noexcept {
    return internal::UnaryOperator(MathOperator::MATH_SIN, std::forward<T>(n));
}

template <typename T>
Node cos(T &&n) noexcept {
    return internal::UnaryOperator(MathOperator::MATH_COS, std::forward<T>(n));
}

template <typename T>
Node tan(T &&n) noexcept {
    return internal::UnaryOperator(MathOperator::MATH_TAN, std::forward<T>(n));
}

template <typename T>
Node asin(T &&n) noexcept {
    return internal::UnaryOperator(MathOperator::MATH_ARCSIN, std::forward<T>(n));
}

template <typename T>
Node acos(T &&n) noexcept {
    return internal::UnaryOperator(MathOperator::MATH_ARCCOS, std::forward<T>(n));
}

template <typename T>
Node atan(T &&n) noexcept {
    return internal::UnaryOperator(MathOperator::MATH_ARCTAN, std::forward<T>(n));
}

template <typename T>
Node sqrt(T &&n) noexcept {
    return internal::UnaryOperator(MathOperator::MATH_SQRT, std::forward<T>(n));
}

template <typename T>
Node log(T &&n) noexcept {
    return internal::UnaryOperator(MathOperator::MATH_LOG, std::forward<T>(n));
}

template <typename T>
Node log2(T &&n) noexcept {
    return internal::UnaryOperator(MathOperator::MATH_LOG2, std::forward<T>(n));
}

template <typename T>
Node log10(T &&n) noexcept {
    return internal::UnaryOperator(MathOperator::MATH_LOG10, std::forward<T>(n));
}

template <typename T>
Node exp(T &&n) noexcept {
    return internal::UnaryOperator(MathOperator::MATH_EXP, std::forward<T>(n));
}

} // namespace tomsolver

namespace tomsolver {

/**
 * node对varname求导。在node包含多个变量时，是对varname求偏导。
 * @exception runtime_error 如果表达式内包含AND(&) OR(|) MOD(%)这类不能求导的运算符，则抛出异常
 */
Node Diff(const Node &node, const std::string &varname, int i = 1);

/**
 * node对varname求导。在node包含多个变量时，是对varname求偏导。
 * @exception runtime_error 如果表达式内包含AND(&) OR(|) MOD(%)这类不能求导的运算符，则抛出异常
 */
Node Diff(Node &&node, const std::string &varname, int i = 1);

} // namespace tomsolver

namespace tomsolver {

class SymVec;
class SymMat {
public:
    /**
     *
     */
    SymMat(int rows, int cols) noexcept;

    /**
     * 使用初始化列表构造。注意列表内的对象将被强行移动至Vec内部。
     */
    SymMat(std::initializer_list<std::initializer_list<Node>> init) noexcept;

    /**
     * 从数值矩阵构造符号矩阵
     */
    SymMat(const Mat &rhs) noexcept;

    SymMat Clone() const noexcept;

    bool Empty() const noexcept;

    int Rows() const noexcept;

    int Cols() const noexcept;

    /**
     * 输出Vec。如果列数不为1，抛出异常。
     * @exception runtime_error 列数不为1
     */
    SymVec ToSymVec() const;

    /**
     * 逐个元素转换为符号向量（列向量）。
     */
    SymVec ToSymVecOneByOne() const noexcept;

    /**
     * 得到数值矩阵。前提条件是矩阵内的元素均为数值节点，否则抛出异常。
     * @exception runtime_error 存在非数值节点
     */
    Mat ToMat() const;

    /**
     * 把矩阵的内的元素均计算为数值节点。
     * @exception runtime_error 如果有变量存在，则无法计算
     * @exception MathError 出现浮点数无效值(inf, -inf, nan)
     */
    SymMat &Calc();

    SymMat &Subs(const std::map<std::string, double> &varValues) noexcept;

    SymMat &Subs(const VarsTable &varsTable) noexcept;

    /**
     * 返回符号矩阵内出现的所有变量名。
     */
    std::set<std::string> GetAllVarNames() const noexcept;

    /**
     * 如果rhs和自己的维数不匹配会触发assert。
     */
    SymMat operator-(const SymMat &rhs) const noexcept;

    /**
     *
     * @exception MathError 维数不匹配
     */
    SymMat operator*(const SymMat &rhs) const;

    /**
     * 返回是否相等。
     * 目前只能判断表达式树完全一致的情况。
     * TODO 改为可以判断等价表达式
     */
    bool operator==(const SymMat &rhs) const noexcept;

    Node &Value(int i, int j) noexcept;
    const Node &Value(int i, int j) const noexcept;

    std::string ToString() const noexcept;

protected:
    int rows, cols;
    std::unique_ptr<std::valarray<Node>> data;

    friend SymMat Jacobian(const SymMat &equations, const std::vector<std::string> &vars) noexcept;
};

class SymVec : public SymMat {
public:
    /**
     *
     */
    SymVec(int rows) noexcept;

    /**
     * 使用初始化列表构造。注意列表内的对象将被强行移动至Vec内部。
     */
    SymVec(std::initializer_list<Node> init) noexcept;

    /**
     * 如果rhs和自己的维数不匹配会触发assert。
     */
    SymVec operator-(const SymVec &rhs) const noexcept;

    Node &operator[](std::size_t index) noexcept;

    const Node &operator[](std::size_t index) const noexcept;
};

SymMat Jacobian(const SymMat &equations, const std::vector<std::string> &vars) noexcept;

std::ostream &operator<<(std::ostream &out, const SymMat &symMat) noexcept;

} // namespace tomsolver

namespace tomsolver {

/**
 * 用newNode节点替换oldVar指定的变量。
 */
Node Subs(const Node &node, const std::string &oldVar, const Node &newNode) noexcept;

/**
 * 用newNode节点替换oldVar指定的变量。
 */
Node Subs(Node &&node, const std::string &oldVar, const Node &newNode) noexcept;

/**
 * 用newNodes节点替换oldVars指定的变量。
 */
Node Subs(const Node &node, const std::vector<std::string> &oldVars, const SymVec &newNodes) noexcept;

/**
 * 用newNodes节点替换oldVars指定的变量。
 */
Node Subs(Node &&node, const std::vector<std::string> &oldVars, const SymVec &newNodes) noexcept;

/**
 * 用newNodes节点替换oldVars指定的变量。
 */
Node Subs(const Node &node, const std::map<std::string, Node> &dict) noexcept;

/**
 * 用newNodes节点替换oldVars指定的变量。
 */
Node Subs(Node &&node, const std::map<std::string, Node> &dict) noexcept;

/**
 * 用newNodes节点替换oldVars指定的变量。
 */
Node Subs(const Node &node, const std::map<std::string, double> &varValues) noexcept;

/**
 * 用newNodes节点替换oldVars指定的变量。
 */
Node Subs(Node &&node, const std::map<std::string, double> &varValues) noexcept;

/**
 * 用newNodes节点替换oldVars指定的变量。
 */
Node Subs(const Node &node, const VarsTable &varsTable) noexcept;

/**
 * 用newNodes节点替换oldVars指定的变量。
 */
Node Subs(Node &&node, const VarsTable &varsTable) noexcept;

} // namespace tomsolver

namespace tomsolver {

namespace internal {

class SubsFunctions {
public:
    // 前序遍历。非递归实现。
    static Node SubsInner(Node node, const std::map<std::string, Node> &dict) noexcept {

        std::stack<std::tuple<NodeImpl &>> stk;

        auto Replace = [&dict](Node &cur) {
            if (cur->type != NodeType::VARIABLE) {
                return false;
            }

            auto itor = dict.find(cur->varname);
            if (itor == dict.end()) {
                return false;
            }

            auto parent = cur->parent;
            cur = Clone(itor->second);
            cur->parent = parent;

            return true;
        };

        if (!Replace(node)) {
            auto TryReplace = [&stk, &Replace](Node &cur) {
                if (cur && !Replace(cur)) {
                    stk.emplace(*cur);
                }
            };

            TryReplace(node->right);
            TryReplace(node->left);

            while (!stk.empty()) {
                auto &[f] = stk.top();
                stk.pop();
                TryReplace(f.right);
                TryReplace(f.left);
            }
        }

#ifndef NDEBUG
        node->CheckParent();
#endif
        return node;
    }
};

} // namespace internal

Node Subs(const Node &node, const std::string &oldVar, const Node &newNode) noexcept {
    return Subs(Clone(node), oldVar, newNode);
}

Node Subs(Node &&node, const std::string &oldVar, const Node &newNode) noexcept {
    std::map<std::string, Node> dict;
    dict.try_emplace(oldVar, Clone(newNode));
    return internal::SubsFunctions::SubsInner(Move(node), dict);
}

Node Subs(const Node &node, const std::vector<std::string> &oldVars, const SymVec &newNodes) noexcept {
    return Subs(Clone(node), oldVars, newNodes);
}

Node Subs(Node &&node, const std::vector<std::string> &oldVars, const SymVec &newNodes) noexcept {
    assert(static_cast<int>(oldVars.size()) == newNodes.Rows());
    std::map<std::string, Node> dict;
    for (size_t i = 0; i < oldVars.size(); ++i) {
        dict.try_emplace(oldVars[i], Clone(newNodes[i]));
    }
    return internal::SubsFunctions::SubsInner(Move(node), dict);
}

Node Subs(const Node &node, const std::map<std::string, Node> &dict) noexcept {
    return Subs(Clone(node), dict);
}

Node Subs(Node &&node, const std::map<std::string, Node> &dict) noexcept {
    return internal::SubsFunctions::SubsInner(Move(node), dict);
}

Node Subs(const Node &node, const std::map<std::string, double> &varValues) noexcept {
    return Subs(Clone(node), varValues);
}

Node Subs(Node &&node, const std::map<std::string, double> &varValues) noexcept {
    std::map<std::string, Node> dict;
    for (auto &[var, val] : varValues) {
        dict.try_emplace(var, Num(val));
    }
    return internal::SubsFunctions::SubsInner(Move(node), dict);
}

Node Subs(const Node &node, const VarsTable &varsTable) noexcept {
    return Subs(Clone(node), varsTable);
}

Node Subs(Node &&node, const VarsTable &varsTable) noexcept {
    std::map<std::string, Node> dict;
    for (auto &[var, val] : varsTable) {
        dict.try_emplace(var, Num(val));
    }
    return internal::SubsFunctions::SubsInner(Move(node), dict);
}

} // namespace tomsolver

namespace tomsolver {

void Simplify(Node &node) noexcept;

} // namespace tomsolver

namespace tomsolver {

namespace internal {

class SimplifyFunctions {
public:
    struct SimplifyNode {
        NodeImpl &node;
        bool isLeftChild;

        SimplifyNode(NodeImpl &node) : node(node), isLeftChild(!node.parent || node.parent->left.get() == &node) {}
    };

    // 对单节点n进行化简。
    static void SimplifySingleNode(std::unique_ptr<NodeImpl> &n) noexcept {
        auto parent = n->parent;
        switch (GetOperatorNum(n->op)) {
        // 对于1元运算符，且儿子是数字的，直接计算出来
        case 1:
            if (n->left->type == NodeType::NUMBER) {
                n->type = NodeType::NUMBER;
                n->value = tomsolver::Calc(n->op, n->left->value, 0);
                n->op = MathOperator::MATH_NULL;
                n->left = nullptr;
            }
            break;

        // 对于2元运算符
        case 2:
            // 儿子是数字的，直接计算出来
            if (n->left->type == NodeType::NUMBER && n->right->type == NodeType::NUMBER) {
                n->type = NodeType::NUMBER;
                n->value = tomsolver::Calc(n->op, n->left->value, n->right->value);
                n->op = MathOperator::MATH_NULL;
                n->left = nullptr;
                n->right = nullptr;
                return;
            }

            assert(n->left && n->right);
            bool lChildIs0 = n->left->type == NodeType::NUMBER && n->left->value == 0.0;
            bool rChildIs0 = n->right->type == NodeType::NUMBER && n->right->value == 0.0;
            bool lChildIs1 = n->left->type == NodeType::NUMBER && n->left->value == 1.0;
            bool rChildIs1 = n->right->type == NodeType::NUMBER && n->right->value == 1.0;

            // 任何数乘或被乘0、被0除、0的除0外的任何次方，等于0
            if ((n->op == MathOperator::MATH_MULTIPLY && (lChildIs0 || rChildIs0)) ||
                (n->op == MathOperator::MATH_DIVIDE && lChildIs0) || (n->op == MathOperator::MATH_POWER && lChildIs0)) {
                n = Num(0);
                n->parent = parent;
                return;
            }

            // 任何数加或被加0、被减0、乘或被乘1、被1除、开1次方，等于自身
            if ((n->op == MathOperator::MATH_ADD && (lChildIs0 || rChildIs0)) ||
                (n->op == MathOperator::MATH_SUB && rChildIs0) ||
                (n->op == MathOperator::MATH_MULTIPLY && (lChildIs1 || rChildIs1)) ||
                (n->op == MathOperator::MATH_DIVIDE && rChildIs1) || (n->op == MathOperator::MATH_POWER && rChildIs1)) {

                if (lChildIs1 || lChildIs0) {
                    n->left = nullptr;
                    n = Move(n->right);
                    n->parent = parent;
                } else if (rChildIs1 || rChildIs0) {
                    n->right = nullptr;
                    n = Move(n->left);
                    n->parent = parent;
                }
                return;
            }
        }
    }

    // 后序遍历。非递归实现。
    static void SimplifyWholeNode(Node &node) {

        // 借助一个栈，得到反向的后序遍历序列，结果保存在revertedPostOrder。除了root节点，root节点不保存在revertedPostOrder里，最后单独化简。
        std::stack<SimplifyNode> stk;
        std::deque<SimplifyNode> revertedPostOrder;

        auto popNode = [&stk] {
            auto node = std::move(stk.top());
            stk.pop();
            return node;
        };

        // ==== Part I ====

        if (node->type != NodeType::OPERATOR) {
            return;
        }

        stk.push(SimplifyNode(*node.get()));

        while (!stk.empty()) {
            auto f = popNode();

            if (f.node.left && f.node.left->type == NodeType::OPERATOR) {
                stk.push(SimplifyNode(*f.node.left.get()));
            }

            if (f.node.right && f.node.right->type == NodeType::OPERATOR) {
                stk.push(SimplifyNode(*f.node.right.get()));
            }

            revertedPostOrder.emplace_back(std::move(f));
        }

        // pop掉root，root最后单独处理
        revertedPostOrder.pop_front();

        // ==== Part II ====
        std::for_each(revertedPostOrder.rbegin(), revertedPostOrder.rend(), [](SimplifyNode &snode) {
            SimplifySingleNode(snode.isLeftChild ? snode.node.parent->left : snode.node.parent->right);
        });

        SimplifyFunctions::SimplifySingleNode(node);

        return;

        // if (GetOperateNum(now->eOperator) == 2) {

        //    //任何节点的0次方均等于1，除了0的0次方已在前面报错
        //    if (now->eOperator == MATH_POWER && RChildIs0) {
        //        //替换掉当前运算符，这个1节点将在回溯时处理
        //        //新建一个1节点
        //        TNode *temp;
        //        temp = new TNode;
        //        temp->eType = NODE_NUMBER;
        //        temp->value = 1;

        //        // 0节点连接到上面
        //        if (now != head) {
        //            if (now->parent->left == now) {
        //                now->parent->left = temp;
        //                temp->parent = now->parent;
        //            }
        //            if (now->parent->right == now) {
        //                now->parent->right = temp;
        //                temp->parent = now->parent;
        //            }
        //        } else
        //            head = temp;

        //        DeleteNode(now);
        //    }

        //    // 0-x=-x
        //    if (now->eOperator == MATH_SUB && LChildIs0) {
        //        TNode *LChild = now->left;
        //        TNode *RChild = now->right;
        //        now->eOperator = MATH_NEGATIVE;
        //        now->left = RChild;
        //        now->right = NULL;

        //        delete LChild;
        //    }

        //}
    }
};

} // namespace internal

void Simplify(Node &node) noexcept {
    internal::SimplifyFunctions::SimplifyWholeNode(node);
    return;
}

} // namespace tomsolver

namespace tomsolver {

namespace internal {
struct Token;
}

class ParseError : public std::runtime_error {
public:
protected:
    ParseError() : std::runtime_error("parse error") {}
};

class SingleParseError : public ParseError {
public:
    template <typename... T>
    SingleParseError(int line, int pos, std::string_view content, T &&...errInfo)
        : line(line), pos(pos), content(content) {
        std::stringstream ss;

        ss << "[Parse Error] ";
        (ss << ... << std::forward<T>(errInfo));
        ss << " at(" << line << ", " << pos << "):\n"
           << content << "\n"
           << std::string(pos, ' ') << "^---- error position";

        whatStr = ss.str();
    }

    virtual const char *what() const noexcept override;

    int GetLine() const noexcept;

    int GetPos() const noexcept;

private:
    int line;                 // 行号
    int pos;                  // 第几个字符
    std::string_view content; // 整行文本
    std::string whatStr;      // 完整的错误信息
};

class MultiParseError : public ParseError {
public:
    MultiParseError(const std::vector<SingleParseError> &parseErrors);

    virtual const char *what() const noexcept override;

private:
    std::vector<SingleParseError> parseErrors;
    std::string whatStr; // 完整的错误信息
};

namespace internal {

struct Token {
    std::string_view s;       // token内容
    int line;                 // 行号
    int pos;                  // 第几个字符
    bool isBaseOperator;      // 是否为基本运算符（单个字符的运算符以及左右括号）
    std::string_view content; // 整行文本
    Node node;                // 节点
    Token(int line, int pos, bool isBaseOperator, std::string_view s, std::string_view content)
        : s(s), line(line), pos(pos), isBaseOperator(isBaseOperator), content(content) {}
};

class ParseFunctions {
public:
    /**
     * 解析表达式字符串为in order记号流。其实就是做词法分析。
     * @exception ParseError
     */
    static std::deque<Token> ParseToTokens(std::string_view expression);

    /**
     * 由in order序列得到post order序列。实质上是把记号流转化为逆波兰表达式。
     * @exception ParseError
     */
    static std::vector<Token> InOrderToPostOrder(std::deque<Token> &inOrder);

    /**
     * 将PostOrder建立为树，并进行表达式有效性检验（确保二元及一元运算符、函数均有操作数）。
     * @exception ParseError
     */
    static Node BuildExpressionTree(std::vector<Token> &postOrder);
};

} // namespace internal

/**
 * 把字符串解析为表达式。
 * @exception ParseError
 */
Node Parse(std::string_view expression);

Node operator""_f(const char *exp, size_t);

} // namespace tomsolver

namespace tomsolver {

namespace {

constexpr auto fnv1a(std::string_view s) {
    constexpr uint64_t offsetBasis = 14695981039346656037ul;
    constexpr uint64_t prime = 1099511628211ul;

    uint64_t hash = offsetBasis;

    for (auto c : s) {
        hash = (hash ^ c) * prime;
    }

    return hash;
}

constexpr auto operator""_fnv1a(const char *s, size_t) {
    return fnv1a(s);
}

/* 是基本运算符()+-* /^&|% */
bool IsBasicOperator(char c) noexcept {
    switch (c) {
    case '(':
    case ')':
    case '+':
    case '-':
    case '*':
    case '/':
    case '^':
    case '&':
    case '|':
    case '%':
        return true;
    }
    return false;
}

/*  */
MathOperator BaseOperatorCharToEnum(char c, bool unary) noexcept {
    switch (c) {
    case '(':
        return MathOperator::MATH_LEFT_PARENTHESIS;
    case ')':
        return MathOperator::MATH_RIGHT_PARENTHESIS;
    case '+':
        return unary ? MathOperator::MATH_POSITIVE : MathOperator::MATH_ADD;
    case '-':
        return unary ? MathOperator::MATH_NEGATIVE : MathOperator::MATH_SUB;
    case '*':
        return MathOperator::MATH_MULTIPLY;
    case '/':
        return MathOperator::MATH_DIVIDE;
    case '^':
        return MathOperator::MATH_POWER;
    case '&':
        return MathOperator::MATH_AND;
    case '|':
        return MathOperator::MATH_OR;
    case '%':
        return MathOperator::MATH_MOD;
    default:
        assert(0);
    }
    assert(0);
    return MathOperator::MATH_NULL;
}

MathOperator Str2Function(std::string_view s) noexcept {
    switch (fnv1a(s)) {
    case "sin"_fnv1a:
        return MathOperator::MATH_SIN;
    case "cos"_fnv1a:
        return MathOperator::MATH_COS;
    case "tan"_fnv1a:
        return MathOperator::MATH_TAN;
    case "arcsin"_fnv1a:
        return MathOperator::MATH_ARCSIN;
    case "arccos"_fnv1a:
        return MathOperator::MATH_ARCCOS;
    case "arctan"_fnv1a:
        return MathOperator::MATH_ARCTAN;
    case "sqrt"_fnv1a:
        return MathOperator::MATH_SQRT;
    case "log"_fnv1a:
        return MathOperator::MATH_LOG;
    case "log2"_fnv1a:
        return MathOperator::MATH_LOG2;
    case "log10"_fnv1a:
        return MathOperator::MATH_LOG10;
    case "exp"_fnv1a:
        return MathOperator::MATH_EXP;
    }
    return MathOperator::MATH_NULL;
}

} // namespace

const char *SingleParseError::what() const noexcept {
    return whatStr.c_str();
}

int SingleParseError::GetLine() const noexcept {
    return line;
}

int SingleParseError::GetPos() const noexcept {
    return pos;
}

MultiParseError::MultiParseError(const std::vector<SingleParseError> &parseErrors) : parseErrors(parseErrors) {
    std::stringstream ss;
    std::transform(parseErrors.rbegin(), parseErrors.rend(), std::ostream_iterator<const char *>(ss, "\n"),
                   [](const auto &err) {
                       return err.what();
                   });
    whatStr = ss.str();
}

const char *MultiParseError::what() const noexcept {
    return whatStr.c_str();
}

namespace internal {

std::deque<Token> ParseFunctions::ParseToTokens(std::string_view content) {

    if (content.empty()) {
        throw SingleParseError(0, 0, "empty input", content);
    }

    auto iter = content.begin(), s = iter;
    std::deque<Token> ret;

    auto tryComfirmToken = [&ret, &iter, &s, &content] {
        if (size_t size = std::distance(s, iter)) {
            auto exp = std::string_view{s, size};
            auto &token = ret.emplace_back(0, std::distance(content.begin(), s), false, exp, content);

            // 检验是否为浮点数
            try {
                std::size_t sz;
                auto expStr = std::string{exp};
                auto d = std::stod(expStr, &sz);
                if (sz == expStr.size()) {
                    token.node = Num(d);
                    return;
                }
            } catch (...) {}

            if (auto op = Str2Function(exp); op != MathOperator::MATH_NULL) {
                token.node = Op(op);
                return;
            }

            // 变量
            // 非运算符、数字、函数
            if (!VarNameIsLegal(exp)) // 变量名首字符需为下划线或字母
            {
                throw SingleParseError(token.line, token.pos, exp, "Invalid variable name: \"", exp, "\"");
            }

            token.node = Var(exp);
        }
    };

    while (iter != content.end()) {
        if (IsBasicOperator(*iter)) {
            tryComfirmToken();
            auto unaryOp = ret.empty() || (ret.back().node->type == NodeType::OPERATOR &&
                                           ret.back().node->op != MathOperator::MATH_RIGHT_PARENTHESIS);
            ret.emplace_back(0, std::distance(content.begin(), iter), true, std::string_view{iter, 1}, content).node =
                Op(BaseOperatorCharToEnum(*iter, unaryOp));
            s = ++iter;
        } else if (isspace(*iter)) {
            // 忽略tab (\t) whitespaces (\n, \v, \f, \r) space
            tryComfirmToken();
            s = ++iter;
        } else {
            ++iter;
        }
    }

    tryComfirmToken();

    return ret;
}

std::vector<Token> ParseFunctions::InOrderToPostOrder(std::deque<Token> &inOrder) {
    std::vector<Token> postOrder;
    int parenthesisBalance = 0;
    std::stack<Token> tokenStack;

    auto popToken = [&tokenStack] {
        auto r = std::move(tokenStack.top());
        tokenStack.pop();
        return r;
    };

    while (!inOrder.empty()) {
        auto f = std::move(inOrder.front());
        inOrder.pop_front();

        switch (f.node->type) {
        // 数字直接入栈
        case NodeType::NUMBER:
        case NodeType::VARIABLE:
            postOrder.emplace_back(std::move(f));
            continue;
        default:
            break;
        };

        switch (f.node->op) {
        case MathOperator::MATH_LEFT_PARENTHESIS:
            parenthesisBalance++;
            break;

        case MathOperator::MATH_POSITIVE:
        case MathOperator::MATH_NEGATIVE:
            break;

        case MathOperator::MATH_RIGHT_PARENTHESIS:
            if (parenthesisBalance == 0) {
                throw SingleParseError(f.line, f.pos, f.content, "Parenthesis not match: \"", f.s, "\"");
            }
            for (auto token = popToken(); token.node->op != MathOperator::MATH_LEFT_PARENTHESIS; token = popToken()) {
                postOrder.emplace_back(std::move(token));
            }
            if (!tokenStack.empty() && IsFunction(tokenStack.top().node->op)) {
                postOrder.emplace_back(popToken());
            }
            while (!tokenStack.empty() && (tokenStack.top().node->op == MathOperator::MATH_POSITIVE ||
                                           tokenStack.top().node->op == MathOperator::MATH_NEGATIVE)) {
                postOrder.emplace_back(popToken());
            }
            continue;

        default:
            // 不是括号也不是正负号
            if (!tokenStack.empty()) {
                auto compare =
                    IsLeft2Right(tokenStack.top().node->op)
                        ? std::function{[cmp = std::less_equal<>{}, rank = Rank(f.node->op)](
                                            const Token &token) { // 左结合，则挤出高优先级及同优先级符号
                              return cmp(rank, Rank(token.node->op));
                          }}
                        : std::function{[cmp = std::less<>{}, rank = Rank(f.node->op)](
                                            const Token &token) { // 右结合，则挤出高优先级，但不挤出同优先级符号
                              return cmp(rank, Rank(token.node->op));
                          }};

                while (!tokenStack.empty() && compare(tokenStack.top())) {
                    postOrder.push_back(std::move(tokenStack.top())); // 符号进入post队列
                    tokenStack.pop();
                }
            }
            break;
        }

        tokenStack.push(std::move(f)); // 高优先级已全部挤出，当前符号入栈
    }

    // 剩下的元素全部入栈
    while (!tokenStack.empty()) {
        auto token = popToken();

        // 退栈时出现左括号，说明没有找到与之匹配的右括号
        if (token.node->op == MathOperator::MATH_LEFT_PARENTHESIS) {
            throw SingleParseError(token.line, token.pos, token.content, "Parenthesis not match: \"", token.s, "\"");
        }

        postOrder.emplace_back(std::move(token));
    }

    return postOrder;
}

// 将PostOrder建立为树，并进行表达式有效性检验（确保二元及一元运算符、函数均有操作数）
Node ParseFunctions::BuildExpressionTree(std::vector<Token> &postOrder) {
    std::stack<Token> tokenStack;
    auto pushToken = [&tokenStack](Token &token) {
        tokenStack.emplace(std::move(token));
    };
    auto popNode = [&tokenStack] {
        auto node = std::move(tokenStack.top().node);
        tokenStack.pop();
        return node;
    };
    // 逐个识别PostOrder序列，构建表达式树
    for (auto &token : postOrder) {
        switch (token.node->type) {
        case NodeType::OPERATOR:
            if (GetOperatorNum(token.node->op) == 2) {
                if (tokenStack.empty()) {
                    throw MathError{ErrorType::ERROR_WRONG_EXPRESSION};
                }

                tokenStack.top().node->parent = token.node.get();
                token.node->right = popNode();

                if (tokenStack.empty()) {
                    throw MathError{ErrorType::ERROR_WRONG_EXPRESSION};
                }

                tokenStack.top().node->parent = token.node.get();
                token.node->left = popNode();

                pushToken(token);
                continue;
            }

            // 一元运算符
            assert(GetOperatorNum(token.node->op) == 1);

            if (tokenStack.empty()) {
                throw MathError{ErrorType::ERROR_WRONG_EXPRESSION};
            }

            tokenStack.top().node->parent = token.node.get();
            token.node->left = popNode();

            break;

        default:
            break;
        }

        pushToken(token);
    }

    // 如果现在临时栈里面有超过1个元素，那么除了栈顶，其他的都代表出错
    if (tokenStack.size() > 1) {
        // 扔掉最顶上的，构造到一半的表达式
        tokenStack.pop();

        std::vector<SingleParseError> errors;
        while (!tokenStack.empty()) {
            Token &token = tokenStack.top();
            errors.emplace_back(token.line, token.pos, token.content, "Parse Error at: \"", token.s, "\"");
            tokenStack.pop();
        }
        throw MultiParseError(errors);
    }

    return popNode();
}

} // namespace internal

Node Parse(std::string_view expression) {
    auto tokens = internal::ParseFunctions::ParseToTokens(expression);
    auto postOrder = internal::ParseFunctions::InOrderToPostOrder(tokens);
    auto node = internal::ParseFunctions::BuildExpressionTree(postOrder);
    return node;
}

Node operator""_f(const char *exp, size_t) {
    return Parse(exp);
}

} // namespace tomsolver

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
VarsTable SolveByNewtonRaphson(const VarsTable &varsTable, const SymVec &equations);

/**
 * 解非线性方程组equations。
 * 初值及变量名通过varsTable传入。
 * @exception runtime_error 迭代次数超出限制
 */
VarsTable SolveByLM(const VarsTable &varsTable, const SymVec &equations);

/**
 * 解非线性方程组equations。
 * 初值及变量名通过varsTable传入。
 * @exception runtime_error 迭代次数超出限制
 */
VarsTable Solve(const VarsTable &varsTable, const SymVec &equations);

/**
 * 解非线性方程组equations。
 * 变量名通过分析equations得到。初值通过Config::get()得到。
 * @exception runtime_error 迭代次数超出限制
 */
VarsTable Solve(const SymVec &equations);

} // namespace tomsolver

using std::cout;
using std::endl;
using std::runtime_error;

namespace tomsolver {

double Armijo(const Vec &x, const Vec &d, std::function<Vec(Vec)> f, std::function<Mat(Vec)> df) {
    double alpha = 1;   // a > 0
    double gamma = 0.4; // 取值范围(0, 0.5)越大越快
    double sigma = 0.5; // 取值范围(0, 1)越大越慢
    Vec x_new(x);
    while (1) {
        x_new = x + alpha * d;

        auto l = f(x_new).Norm2();
        auto r = (f(x).AsMat() + gamma * alpha * df(x).Transpose() * d).Norm2();
        if (l <= r) // 检验条件
        {
            break;
        } else
            alpha = alpha * sigma; // 缩小alpha，进入下一次循环
    }
    return alpha;
}

double FindAlpha(const Vec &x, const Vec &d, std::function<Vec(Vec)> f, double uncert) {
    double alpha_cur = 0;

    double alpha_new = 1;

    int it = 0;
    int maxIter = 100;

    Vec g_cur = f(x + alpha_cur * d);

    while (std::abs(alpha_new - alpha_cur) > alpha_cur * uncert) {
        double alpha_old = alpha_cur;
        alpha_cur = alpha_new;
        Vec g_old = g_cur;
        g_cur = f(x + alpha_cur * d);

        if (g_cur < g_old) {
            break;
        }

        // FIXME: nan occurred
        alpha_new = EachDivide((g_cur * alpha_old - g_old * alpha_cur), (g_cur - g_old)).NormNegInfinity();

        // cout << it<<"\t"<<alpha_new << endl;
        if (it++ > maxIter) {
            cout << "FindAlpha: over iterator" << endl;
            break;
        }
    }
    return alpha_new;
}

VarsTable SolveByNewtonRaphson(const VarsTable &varsTable, const SymVec &equations) {
    int it = 0; // 迭代计数
    VarsTable table = varsTable;
    int n = table.VarNums(); // 未知量数量
    Vec q(n);                // x向量

    SymMat jaEqs = Jacobian(equations, table.Vars());

    if (Config::get().logLevel >= LogLevel::TRACE) {
        cout << "Jacobian = " << jaEqs.ToString() << endl;
    }

    while (1) {
        Vec phi = equations.Clone().Subs(table).Calc().ToMat().ToVec();
        if (Config::get().logLevel >= LogLevel::TRACE) {
            cout << "iteration = " << it << endl;
            cout << "phi = " << phi << endl;
        }

        if (phi == 0) {
            break;
        }

        if (it > Config::get().maxIterations) {
            throw runtime_error("迭代次数超出限制");
        }

        Mat ja = jaEqs.Clone().Subs(table).Calc().ToMat();

        Vec deltaq = SolveLinear(ja, -phi);

        q += deltaq;

        if (Config::get().logLevel >= LogLevel::TRACE) {
            cout << "ja = " << ja << endl;
            cout << "deltaq = " << deltaq << endl;
            cout << "q = " << q << endl;
        }

        table.SetValues(q);

        ++it;
    }
    return table;
}

VarsTable SolveByLM(const VarsTable &varsTable, const SymVec &equations) {
    int it = 0; // 迭代计数
    VarsTable table = varsTable;
    int n = table.VarNums(); // 未知量数量
    Vec q = table.Values();  // x向量

    SymMat JaEqs = Jacobian(equations, table.Vars());

    if (Config::get().logLevel >= LogLevel::TRACE) {
        cout << "Jacobi = " << JaEqs << endl;
    }

    while (1) {
        if (Config::get().logLevel >= LogLevel::TRACE) {
            cout << "iteration = " << it << endl;
        }

        double mu = 1e-5; // LM方法的λ值

        Vec F = equations.Clone().Subs(table).Calc().ToMat().ToVec(); // 计算F

        if (Config::get().logLevel >= LogLevel::TRACE) {
            cout << "F = " << F << endl;
        }

        if (F == 0) { // F值为0，满足方程组求根条件
            break;
        }

        Vec FNew(n);   // 下一轮F
        Vec deltaq(n); // Δq
        while (1) {

            Mat J = JaEqs.Clone().Subs(table).Calc().ToMat(); // 计算雅可比矩阵

            if (Config::get().logLevel >= LogLevel::TRACE) {
                cout << "J = " << J << endl;
            }

            // 说明：
            // 标准的LM方法中，d=-(J'*J+λI)^(-1)*J'F，其中J'*J是为了确保矩阵对称正定。有时d会过大，很难收敛。
            // 牛顿法的 d=-(J+λI)^(-1)*F

            // 方向向量
            Vec d = SolveLinear(J.Transpose() * J + mu * Mat(J.Rows(), J.Cols()).Ones(),
                                -(J.Transpose() * F).ToVec()); // 得到d

            if (Config::get().logLevel >= LogLevel::TRACE) {
                cout << "d = " << d << endl;
            }

            double alpha = Armijo(
                q, d,
                [&](Vec v) -> Vec {
                    table.SetValues(v);
                    return equations.Clone().Subs(table).Calc().ToMat().ToVec();
                },
                [&](Vec v) -> Mat {
                    table.SetValues(v);
                    return JaEqs.Clone().Subs(table).Calc().ToMat();
                }); // 进行1维搜索得到alpha

            // double alpha = FindAlpha(q, d, std::bind(SixBarAngPosition, std::placeholders::_1, thetaCDKL, Hhit));

            // for (size_t i = 0; i < alpha.rows; ++i)
            //{
            //	if (alpha[i] != alpha[i])
            //		alpha[i] = 1.0;
            //}

            deltaq = alpha * d; // 计算Δq

            Vec qTemp = q + deltaq;
            table.SetValues(qTemp);

            FNew = equations.Clone().Subs(table).Calc().ToMat().ToVec(); // 计算新的F

            if (Config::get().logLevel >= LogLevel::TRACE) {
                cout << "it=" << it << endl;
                cout << "\talpha=" << alpha << endl;
                cout << "mu=" << mu << endl;
                cout << "F.Norm2()=" << F.Norm2() << endl;
                cout << "FNew.Norm2()=" << FNew.Norm2() << endl;
                cout << "\tF(x k+1).Norm2()\t" << ((FNew.Norm2() < F.Norm2()) ? "<" : ">=") << "\tF(x k).Norm2()\t"
                     << endl;
            }

            if (FNew.Norm2() < F.Norm2()) // 满足下降条件，跳出内层循环
            {
                break;
            } else {
                mu *= 10.0; // 扩大λ，使模型倾向梯度下降方向
            }

            if (it++ == Config::get().maxIterations) {
                throw runtime_error("迭代次数超出限制");
            }
        }

        q += deltaq; // 应用Δq，更新q值

        table.SetValues(q);

        F = FNew; // 更新F

        if (it++ == Config::get().maxIterations) {
            throw runtime_error("迭代次数超出限制");
        }

        if (Config::get().logLevel >= LogLevel::TRACE) {
            cout << std::string(20, '=') << endl;
        }
    }

    if (Config::get().logLevel >= LogLevel::TRACE) {
        cout << "success" << endl;
    }

    return table;
}

VarsTable Solve(const VarsTable &varsTable, const SymVec &equations) {
    switch (Config::get().nonlinearMethod) {
    case NonlinearMethod::NEWTON_RAPHSON:
        return SolveByNewtonRaphson(varsTable, equations);
    case NonlinearMethod::LM:
        return SolveByLM(varsTable, equations);
    }
    throw runtime_error("invalid config.NonlinearMethod value: " +
                        std::to_string(static_cast<int>(Config::get().nonlinearMethod)));
}

VarsTable Solve(const SymVec &equations) {
    auto varNames = equations.GetAllVarNames();
    std::vector<std::string> vecVarNames(varNames.begin(), varNames.end());
    VarsTable varsTable(std::move(vecVarNames), Config::get().initialValue);
    return Solve(varsTable, equations);
}

} // namespace tomsolver

namespace tomsolver {

namespace internal {

class DiffFunctions {
public:
    struct DiffNode {
        NodeImpl &node;
        const bool isLeftChild;

        DiffNode(NodeImpl &node) : node(node), isLeftChild(node.parent && node.parent->left.get() == &node) {}
    };

    static void DiffOnce(Node &root, const std::string &varname) {
        std::queue<DiffNode> q;

        if (root->type == NodeType::OPERATOR) {
            DiffOnceOperator(root, q);
        } else {
            q.emplace(*root);
        }

        while (!q.empty()) {
            auto &[node, isLeftChild] = q.front();
            q.pop();

            switch (node.type) {
            case NodeType::VARIABLE:
                node.type = NodeType::NUMBER;
                node.value = node.varname == varname ? 1 : 0;
                node.varname = "";
                break;

            case NodeType::NUMBER:
                node.value = 0;
                break;

            case NodeType::OPERATOR: {
                auto &child = isLeftChild ? node.parent->left : node.parent->right;
                DiffOnceOperator(child, q);
                break;
            }
            default:
                assert(0 && "inner bug");
            }
        }
    }

    static void DiffOnceOperator(Node &node, std::queue<DiffNode> &q) {
        auto parent = node->parent;

        // 调用前提：node是1元操作符
        // 如果node的成员是数字，那么整个node变为数字节点，value=0，且返回true
        // 例如： sin(1)' = 0
        auto CullNumberMember = [&node]() -> bool {
            assert(GetOperatorNum(node->op) == 1);
            assert(node->left);
            if (node->left->type == NodeType::NUMBER) {
                node->left = nullptr;
                node->type = NodeType::NUMBER;
                node->op = MathOperator::MATH_NULL;
                node->value = 0.0;
                return true;
            }
            return false;
        };

        // 调用前提：node是2元操作符
        // 如果node的成员是数字，那么整个node变为数字节点，value=0，且返回true
        // 例如： (2*3)' = 0
        auto CullNumberMemberBinary = [&node]() -> bool {
            assert(GetOperatorNum(node->op) == 2);
            assert(node->left && node->right);
            if (node->left->type == NodeType::NUMBER && node->right->type == NodeType::NUMBER) {
                node->left = nullptr;
                node->right = nullptr;
                node->type = NodeType::NUMBER;
                node->op = MathOperator::MATH_NULL;
                node->value = 0.0;
                return true;
            }
            return false;
        };

        switch (node->op) {
        case MathOperator::MATH_NULL: {
            assert(0 && "inner bug");
            break;
        }
        case MathOperator::MATH_POSITIVE:
        case MathOperator::MATH_NEGATIVE: {
            q.emplace(*node->left);
            return;
        }

        // 函数
        case MathOperator::MATH_SIN: {
            if (CullNumberMember()) {
                return;
            }

            // sin(u)' = cos(u) * u'
            node->op = MathOperator::MATH_COS;
            auto u2 = Clone(node->left);
            q.emplace(*u2);
            node = Move(node) * Move(u2);
            node->parent = parent;
            break;
        }
        case MathOperator::MATH_COS: {
            if (CullNumberMember()) {
                return;
            }

            // cos(u)' = -sin(u) * u'
            node->op = MathOperator::MATH_SIN;
            auto u2 = Clone(node->left);
            q.emplace(*u2);
            node = -Move(node) * Move(u2);
            node->parent = parent;
            break;
        }
        case MathOperator::MATH_TAN: {
            if (CullNumberMember()) {
                return;
            }

            // tan'u = 1/(cos(u)^2) * u'
            node->op = MathOperator::MATH_COS;
            auto &u = node->left;
            auto u2 = Clone(u);
            q.emplace(*u2);
            node = Num(1) / (Move(node) ^ Num(2)) * Move(u2);
            node->parent = parent;
            return;
        }
        case MathOperator::MATH_ARCSIN: {
            if (CullNumberMember()) {
                return;
            }

            // asin'u = 1/sqrt(1-u^2) * u'
            auto &u = node->left;
            auto u2 = Clone(u);
            q.emplace(*u2);
            node = (Num(1) / sqrt(Num(1) - (Move(u) ^ Num(2)))) * Move(u2);
            node->parent = parent;
            return;
        }
        case MathOperator::MATH_ARCCOS: {
            if (CullNumberMember()) {
                return;
            }

            // acos'u = -1/sqrt(1-u^2) * u'
            auto &u = node->left;
            auto u2 = Clone(u);
            q.emplace(*u2);
            node = (Num(-1) / sqrt(Num(1) - (Move(u) ^ Num(2)))) * Move(u2);
            node->parent = parent;
            return;
        }
        case MathOperator::MATH_ARCTAN: {
            if (CullNumberMember()) {
                return;
            }

            // atan'u = 1/(1+u^2) * u'
            auto &u = node->left;
            auto u2 = Clone(u);
            q.emplace(*u2);
            node = (Num(1) / (Num(1) + (Move(u) ^ Num(2)))) * Move(u2);
            node->parent = parent;
            return;
        }
        case MathOperator::MATH_SQRT: {
            if (CullNumberMember()) {
                return;
            }

            // sqrt(u)' = 1/(2*sqrt(u)) * u'
            auto &u = node->left;
            auto u2 = Clone(u);
            q.emplace(*u2);
            node = Num(1) / (Num(2) * Move(node)) * Move(u2);
            node->parent = parent;
            return;
        }
        case MathOperator::MATH_LOG: {
            if (CullNumberMember()) {
                return;
            }

            // ln(u)' = 1/u * u'
            auto &u = node->left;
            auto u2 = Clone(u);
            q.emplace(*u2);
            node = (Num(1) / Move(u)) * Move(u2);
            node->parent = parent;
            return;
        }
        case MathOperator::MATH_LOG2: {
            if (CullNumberMember()) {
                return;
            }

            // loga(u)' = 1/(u * ln(a)) * u'
            auto a = 2.0;
            auto &u = node->left;
            auto u2 = Clone(u);
            q.emplace(*u2);
            node = (Num(1) / (Move(u) * Num(std::log(a)))) * Move(u2);
            node->parent = parent;
            return;
        }
        case MathOperator::MATH_LOG10: {
            if (CullNumberMember()) {
                return;
            }

            // loga(u)' = 1/(u * ln(a)) * u'
            auto a = 10.0;
            auto &u = node->left;
            auto u2 = Clone(u);
            q.emplace(*u2);
            node = (Num(1) / (Move(u) * Num(std::log(a)))) * Move(u2);
            node->parent = parent;
            return;
        }
        case MathOperator::MATH_EXP: {
            if (CullNumberMember()) {
                return;
            }

            // e^x=e^x
            if (node->left->type == NodeType::VARIABLE)
                return;

            // (e^u)' = e^u * u'
            auto u2 = Clone(node->left);
            q.emplace(*u2);
            node = Move(node) * Move(u2);
            node->parent = parent;
            break;
        }

        // 二元
        case MathOperator::MATH_ADD:
        case MathOperator::MATH_SUB:
            if (CullNumberMemberBinary()) {
                return;
            }
            // (u + v)' = u' + v'
            if (node->left) {
                q.emplace(*node->left);
            }
            if (node->right) {
                q.emplace(*node->right);
            }
            return;
        case MathOperator::MATH_MULTIPLY: {
            // 两个操作数中有一个是数字
            if (node->left->type == NodeType::NUMBER) {
                q.emplace(*node->right);
                return;
            }
            if (node->right->type == NodeType::NUMBER) {
                q.emplace(*node->left);
                return;
            }

            if (CullNumberMemberBinary()) {
                return;
            }

            // (u*v)' = u' * v + u * v'
            auto &u = node->left;
            auto &v = node->right;
            q.emplace(*u);
            auto u2 = Clone(u);
            auto v2 = Clone(v);
            q.emplace(*v2);
            node = Move(node) + Move(u2) * Move(v2);
            node->parent = parent;
            return;
        }
        case MathOperator::MATH_DIVIDE: {
            // auto leftIsNumber = node->left->type == NodeType::NUMBER;
            auto rightIsNumber = node->right->type == NodeType::NUMBER;

            // f(x)/number = f'(x)/number
            if (rightIsNumber) {
                q.emplace(*node->left);
                return;
            }

            if (CullNumberMemberBinary()) {
                return;
            }

            // (u/v)' = (u'v - uv')/(v^2)
            auto &u = node->left;
            auto &v = node->right;
            auto u2 = Clone(u);
            auto v2 = Clone(v);
            auto v3 = Clone(v);
            q.emplace(*u);
            q.emplace(*v2);
            node = (Move(u) * Move(v) - Move(u2) * Move(v2)) / (Move(v3) ^ Num(2));
            node->parent = parent;
            return;
        }
        case MathOperator::MATH_POWER: {
            // 如果两个操作数都是数字
            if (CullNumberMemberBinary()) {
                return;
            }

            auto lChildIsNumber = node->left->type == NodeType::NUMBER;
            auto rChildIsNumber = node->right->type == NodeType::NUMBER;

            // (u^a)' = a*u^(a-1) * u'
            if (rChildIsNumber) {
                auto &a = node->right;
                auto aValue = a->value;
                auto &u = node->left;
                auto u2 = Clone(u);
                q.emplace(*u2);
                node = std::move(a) * (std::move(u) ^ Num(aValue - 1)) * std::move(u2);
                node->parent = parent;
                return;
            }

            // (a^x)' = a^x * ln(a)  when a>0 and a!=1
            if (lChildIsNumber) {
                auto &a = node->left;
                auto aValue = a->value;
                auto &u = node->right;
                auto u2 = Clone(u);
                q.emplace(*u2);
                node = (std::move(a) ^ std::move(u)) * log(Num(aValue)) * std::move(u2);
                node->parent = parent;
                return;
            }

            // (u^v)' = ( e^(v*ln(u)) )' = e^(v*ln(u)) * (v*ln(u))' = u^v * (v*ln(u))'
            // 左右都不是数字
            auto &u = node->left;
            auto &v = node->right;
            auto vln_u = Clone(v) * log(Clone(u));
            q.emplace(*vln_u);
            node = Move(node) * std::move(vln_u);
            node->parent = parent;
            return;
        }

        case MathOperator::MATH_AND: {
            throw std::runtime_error("can not apply diff for AND(&) operator");
            return;
        }
        case MathOperator::MATH_OR: {
            throw std::runtime_error("can not apply diff for OR(|) operator");
            return;
        }
        case MathOperator::MATH_MOD: {
            throw std::runtime_error("can not apply diff for MOD(%) operator");
            return;
        }
        case MathOperator::MATH_LEFT_PARENTHESIS:
        case MathOperator::MATH_RIGHT_PARENTHESIS:
            assert(0 && "inner bug");
            return;
        default:
            assert(0 && "inner bug");
            return;
        }
    }
};

} // namespace internal

Node Diff(const Node &node, const std::string &varname, int i) {
    auto node2 = Clone(node);
    return Diff(std::move(node2), varname, i);
}

Node Diff(Node &&node, const std::string &varname, int i) {
    assert(i > 0);
    auto n = std::move(node);
    while (i--) {
        internal::DiffFunctions::DiffOnce(n, varname);
    }
#ifndef NDEBUG
    auto s = n->ToString();
    n->CheckParent();
#endif
    Simplify(n);
#ifndef NDEBUG
    n->CheckParent();
#endif
    return n;
}

} // namespace tomsolver

namespace tomsolver {

using DataType = std::valarray<Node>;

SymMat::SymMat(int rows, int cols) noexcept : rows(rows), cols(cols) {
    assert(rows > 0 && cols > 0);
    data.reset(new DataType(rows * cols));
}

SymMat::SymMat(std::initializer_list<std::initializer_list<Node>> init) noexcept {
    rows = static_cast<int>(init.size());
    cols = static_cast<int>(std::max(init, [](auto lhs, auto rhs) {
                                return lhs.size() < rhs.size();
                            }).size());
    data.reset(new DataType(rows * cols));

    auto i = 0;
    for (auto val : init) {
        auto j = 0;
        for (auto &node : val) {
            (*data)[i * cols + j++] = std::move(const_cast<Node &>(node));
        }
        i++;
    }
}

SymMat::SymMat(const Mat &rhs) noexcept : SymMat(rhs.Rows(), rhs.Cols()) {
    std::generate(std::begin(*data), std::end(*data), [p = std::addressof(rhs.Value(0, 0))]() mutable {
        return Num(*p++);
    });
}

SymMat SymMat::Clone() const noexcept {
    SymMat ret(Rows(), Cols());
    std::generate(std::begin(*ret.data), std::end(*ret.data), [iter = std::begin(*data)]() mutable {
        return tomsolver::Clone(*iter++);
    });
    return ret;
}

bool SymMat::Empty() const noexcept {
    return data->size() == 0;
}

int SymMat::Rows() const noexcept {
    return rows;
}

int SymMat::Cols() const noexcept {
    return cols;
}

SymVec SymMat::ToSymVec() const {
    assert(rows > 0);
    if (cols != 1) {
        throw std::runtime_error("SymMat::ToSymVec fail. rows is not one");
    }
    return ToSymVecOneByOne();
}

SymVec SymMat::ToSymVecOneByOne() const noexcept {
    SymVec v(rows * cols);
    std::generate(std::begin(*v.data), std::end(*v.data), [iter = std::begin(*data)]() mutable {
        return tomsolver::Clone(*iter++);
    });
    return v;
}

Mat SymMat::ToMat() const {
    std::valarray<double> newData(data->size());
    std::generate(std::begin(newData), std::end(newData), [iter = std::begin(*data)]() mutable {
        if ((**iter).type != NodeType::NUMBER) {
            throw std::runtime_error("ToMat error: node is not number");
        }
        return (**iter++).value;
    });
    return {rows, cols, newData};
}

SymMat &SymMat::Calc() {
    for (auto &node : *data) {
        node->Calc();
    }
    return *this;
}

SymMat &SymMat::Subs(const std::map<std::string, double> &varValues) noexcept {
    for (auto &node : *data) {
        node = tomsolver::Subs(std::move(node), varValues);
    }
    return *this;
}

SymMat &SymMat::Subs(const VarsTable &varsTable) noexcept {
    for (auto &node : *data) {
        node = tomsolver::Subs(std::move(node), varsTable);
    }
    return *this;
}

std::set<std::string> SymMat::GetAllVarNames() const noexcept {
    std::set<std::string> ret;
    for (auto &node : *data) {
        ret.merge(node->GetAllVarNames());
    }
    return ret;
}

SymMat SymMat::operator-(const SymMat &rhs) const noexcept {
    assert(rhs.rows == rows && rhs.cols == cols);
    SymMat ret(rows, cols);
    std::generate(std::begin(*ret.data), std::end(*ret.data),
                  [lhsIter = std::begin(*data), rhsIter = std::begin(*rhs.data)]() mutable {
                      return *lhsIter++ - *rhsIter++;
                  });
    return ret;
}

SymMat SymMat::operator*(const SymMat &rhs) const {
    if (cols != rhs.rows) {
        throw MathError(ErrorType::SIZE_NOT_MATCH);
    }

    SymMat ans(rows, rhs.cols);
    for (int i = 0; i < Rows(); ++i) {
        for (int j = 0; j < rhs.cols; ++j) {
            auto sum = Value(i, 0) * rhs.Value(0, j);
            for (int k = 1; k < cols; ++k) {
                sum += Value(i, k) * rhs.Value(k, j);
            }
            ans.Value(i, j) = Move(sum);
        }
    }
    return ans;
}

bool SymMat::operator==(const SymMat &rhs) const noexcept {
    if (rhs.rows != rows || rhs.cols != cols) {
        return false;
    }
    return std::equal(std::begin(*data), std::end(*data), std::begin(*rhs.data), [](auto &node1, auto &node2) {
        return node1->Equal(node2);
    });
}

Node &SymMat::Value(int i, int j) noexcept {
    return (*data)[i * cols + j];
}

const Node &SymMat::Value(int i, int j) const noexcept {
    return (*data)[i * cols + j];
}

std::string SymMat::ToString() const noexcept {
    if (data->size() == 0) {
        return "[]";
    }

    std::stringstream ss;
    ss << "[";

    size_t i = 0;
    for (auto &node : *data) {
        ss << (i == 0 ? "" : " ") << node->ToString();
        i++;
        ss << (i % cols == 0 ? (i == data->size() ? "]" : "\n") : ", ");
    }

    return ss.str();
}

SymVec::SymVec(int rows) noexcept : SymMat(rows, 1) {}

SymVec::SymVec(std::initializer_list<Node> init) noexcept : SymMat(static_cast<int>(init.size()), 1) {
    auto i = 0;
    for (auto &node : init) {
        (*data)[i++] = std::move(const_cast<Node &>(node));
    }
}

SymVec SymVec::operator-(const SymVec &rhs) const noexcept {
    return SymMat::operator-(rhs).ToSymVec();
}

Node &SymVec::operator[](std::size_t index) noexcept {
    return (*data)[index];
}

const Node &SymVec::operator[](std::size_t index) const noexcept {
    return (*data)[index];
}

SymMat Jacobian(const SymMat &equations, const std::vector<std::string> &vars) noexcept {
    int rows = equations.rows;
    int cols = static_cast<int>(vars.size());
    SymMat ja(rows, cols);
    std::generate(std::begin(*ja.data), std::end(*ja.data),
                  [iter = std::begin(*equations.data), &vars, i = size_t{0}]() mutable {
                      if (i == vars.size()) {
                          i = 0;
                          iter++;
                      }
                      return Diff(*iter, vars[i++]);
                  });
    return ja;
}

std::ostream &operator<<(std::ostream &out, const SymMat &symMat) noexcept {
    return out << symMat.ToString();
}

} // namespace tomsolver

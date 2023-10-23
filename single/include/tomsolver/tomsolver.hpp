#pragma once

#define _USE_MATH_DEFINES
#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <math.h>
#include <memory>
#include <queue>
#include <regex>
#include <set>
#include <stack>
#include <stdexcept>
#include <string>
#include <type_traits>
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
    MathError(ErrorType errorType, const std::string &extInfo)
        : errorType(errorType), errInfo(GetErrorInfo(errorType) + ": \"" + extInfo + "\"") {}

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

    // 添加新的选项务必同步到Reset方法！

    Config();

    void Reset() noexcept;

    const char *GetDoubleFormatStr() const noexcept;

private:
    char doubleFormatStr[16] = "%.16f";
};

std::string ToString(double value) noexcept;

Config &GetConfig() noexcept;

} // namespace tomsolver

namespace tomsolver {

std::string ToString(double value) noexcept {
    char buf[64];
    int ret = -1;

    // 绝对值过大 或者 绝对值过小，应该使用科学计数法来表示
    if ((std::abs(value) >= 1.0e16 || std::abs(value) <= 1.0e-16) && value != 0.0) {
        ret = snprintf(buf, sizeof(buf), "%.16e", value);

        int state = 0;
        int stripPos = sizeof(buf) - 1;
        int ePos = sizeof(buf) - 1;
        for (int i = ret - 1; i >= 0; --i) {
            switch (state) {
            case 0:
                if (buf[i] == 'e') {
                    ePos = i;
                    state = 1;
                    break;
                }
                break;
            case 1:
                if (buf[i] == '0') {
                    stripPos = i;
                    continue;
                }
                if (buf[i] == '.') {
                    stripPos = i;
                }
                goto end_of_scientific_loop;
            }
        }
    end_of_scientific_loop:
        // 如果没有尾后0
        if (stripPos == sizeof(buf) - 1) {
            return buf;
        }

        if (stripPos > ePos)
            assert(0 && "err: stripPos>ePos");
        int i = stripPos;
        int j = ePos;
        while (1) {
            buf[i] = buf[j];

            if (buf[j] == 0) {
                break;
            }

            ++i;
            ++j;
        }

        return buf;
    } else {
        ret = snprintf(buf, sizeof(buf), GetConfig().GetDoubleFormatStr(), value);
    }

    int stripPos = sizeof(buf) - 1;
    for (int i = ret - 1; i >= 0; --i) {
        if (buf[i] == '0') {
            stripPos = i;
            continue;
        }
        if (buf[i] == '.') {
            stripPos = i;
            break;
        }
        break;
    }
    buf[stripPos] = 0;
    return buf;
}

Config &GetConfig() noexcept {
    static Config config;
    return config;
}

Config::Config() {
    Reset();
}

void Config::Reset() noexcept {
    throwOnInvalidValue = true;
    epsilon = 1.0e-9;
    logLevel = LogLevel::WARN;
    maxIterations = 100;
    nonlinearMethod = NonlinearMethod::NEWTON_RAPHSON;
    initialValue = 1.0;
    allowIndeterminateEquation = false;
}

const char *Config::GetDoubleFormatStr() const noexcept {
    return doubleFormatStr;
}

} // namespace tomsolver
// #pragma once
//
// #include "TVariableTable.h"
// #include "error_type.h"
//
// #include <queue>
// #include <stack>
//
//  namespace tomsolver {
//
//  class TExpressionTree {
//  private:
// #define MAX_VAR_NAME 32 //同时也是浮点数转字符串的最大长度
// #define MIN_DOUBLE 1e-6
//
//     void Release();
//
//     static std::string EnumOperatorToTChar(enumMathOperator eOperator);
//
//     bool isBaseOperator(char c);
//     bool isDoubleChar(char c);
//
//     static enumMathOperator BaseOperatorCharToEnum(char c);
//     static enumMathOperator Str2Function(std::string s);
//
//     TVariableTable *pVariableTable;
//     int iVarAppearedCount;
//     TNode *LastVarNode;
//     // TVariableTable SelfVariableTable;
//     void InQueue2PostQueue(std::queue<TNode *> &InOrder, std::vector<TNode *> &PostOrder);
//     void ReadToInOrder(std::string expression, std::queue<TNode *> &InOrder);
//     static std::string Node2Str(const TNode &node);
//     void BuildExpressionTree(std::vector<TNode *> &PostOrder);
//     void Simplify(TNode *now);
//     void GetNodeNum(TNode *now, int &n);
//     int GetNodeNum(TNode *head);
//     void DeleteNode(TNode *node);
//     void DeleteNodeTraversal(TNode *node);
//     TNode *CopyNodeTree(TNode *oldNode);
//     // std::string  FindVariableTableFrom(const std::string varstr, std::vector<std::string >
//     // newVariableTable);//查找变量是否在变量表中，没有则返回NULL
//     void GetVariablePos(TNode *now, const std::string var, std::vector<TNode *> &VarsPos);
//     void GetVariablePos(const std::string var, std::vector<TNode *> &VarsPos);
//     void CopyVariableTable(std::vector<std::string> &Dest, const std::vector<std::string> source);
//     // void ReplaceNodeVariable(TNode *now, std::vector<std::string > &newVariableTable);
//     bool CanCalc(TNode *now);
//     void LinkParent(TNode *child, TNode *ignore);
//     TNode *NewNode(enumNodeType eType, enumMathOperator eOperator = MATH_NULL);
//     void ReleaseVectorTNode(std::vector<TNode *> vec);
//     void Vpa_inner(TNode *now);
//     void Solve(TNode *now, TNode *&write_pos);
//     void CheckOnlyOneVar(TNode *now);
//     void Subs_inner(TNode *node, std::string ptVar, double value);
//
//  public:
//     TNode *head;
//     void Reset();
//     void Vpa(bool bOutput);
//     void LinkVariableTable(TVariableTable *p); //链接变量表
//     void Read(const std::string expression, bool bOutput);
//     void Read(double num, bool bOutput); //读入只有1个数字的表达式
//
//     //所有操作符未完成
//     std::string Solve(std::string &var,
//                       double &value); //求解单变量方程 不验证可求解性，需提前调用HasOnlyOneVar确认 不改动表达式内容
//     std::string OutputStr();
//     void Simplify(bool bOutput);                            //化简
//     std::string Diff(std::string var, int n, bool bOutput); //对变量求导
//     void Subs(std::string ptVar, double value, bool output);
//     void Subs(const std::string vars, const std::string nums,
//               bool output); // vars为被替换变量，nums为替换表达式，以空格分隔
//     void Subs(std::vector<std::string> VarsVector, std::vector<double> NumsVector, bool output);
//     bool CanCalc();                     //检查是否还有变量存在，可以计算则返回true
//     bool IsSingleVar();                 //检查是否为一元(not used)
//     bool HasOnlyOneVar();               //只有一个变量（只有刚read才有效）
//     bool CheckOnlyOneVar();             //只有一个变量（实时验证）
//     double Value(bool operateHeadNode); //不验证可计算性，必须与CanCalc合用
//     std::string
//     Calc(double *result = NULL); //计算表达式的值，若传入了result则把结果存入。返回值为结果字符串或表达式串。
//
//     TExpressionTree &operator=(const TExpressionTree &expr);
//     TExpressionTree &operator+(const TExpressionTree &expr);
//     TExpressionTree &operator*(double value);
//     TExpressionTree &operator+(double value);
//     TExpressionTree &operator-(double value);
//
//     TExpressionTree();
//     ~TExpressionTree();
// };
//
// } // namespace tomsolver
// #include "TExpressionTree.h"
//
// #include "error_type.h"
//
// #include <string>
// #define _USE_MATH_DEFINES
// #include <math.h>
//
// #include <cassert>
//
//
//
//
//
//
///*  */
// std::string TExpressionTree::EnumOperatorToTChar(TExpressionTree::enumMathOperator eOperator)
//{
//	switch (eOperator)
//	{
//	case MATH_POSITIVE:
//		return "+";
//	case MATH_NEGATIVE:
//		return "-";
//	case MATH_LEFT_PARENTHESIS:
//		return "(";
//	case MATH_RIGHT_PARENTHESIS:
//		return "";
//	case MATH_ADD:
//		return "+";
//	case MATH_SUB:
//		return "-";
//	case MATH_MULTIPLY:
//		return "*";
//	case MATH_DIVIDE:
//		return "/";
//	case MATH_POWER:
//		return "^";
//	case MATH_AND:
//		return "&";
//	case MATH_OR:
//		return "|";
//	case MATH_MOD:
//		return "%";
//	default:
//		throw MathError{ ErrorType::ERROR_WRONG_MATH_OPERATOR, std::string("value:" + ToString(eOperator)) };
//	}
//}
//
//
//
//
//
// void TExpressionTree::Simplify(TNode *now)
//{
//	//左遍历
//	if (now->left != NULL)
//		Simplify(now->left);
//
//	//右遍历
//	if (now->right != NULL)
//		Simplify(now->right);
//
//	//化简
//	//OutputStr();
//	if (GetOperateNum(now->eOperator) == 1)
//	{
//		bool ChildIs0 = (now->left->eType == NODE_NUMBER && std::abs(now->left->value) < MIN_DOUBLE);
//		bool ChildIs1 = (now->left->eType == NODE_NUMBER && std::abs(now->left->value - 1) < MIN_DOUBLE);
//
//		//sin(0)=0
//		if (now->eOperator == MATH_SIN && ChildIs0)
//		{
//			LinkParent(now->left, now);
//			now->left->value = 0;
//			delete now;
//		}
//
//		//cos(0)=1
//		if (now->eOperator == MATH_COS && ChildIs0)
//		{
//			LinkParent(now->left, now);
//			now->left->value = 1;
//			delete now;
//		}
//
//		if (now->eOperator == MATH_NEGATIVE && now->left->eType == NODE_NUMBER)
//		{
//			TNode *negative = now;
//			TNode *num = now->left;
//			LinkParent(num, negative);
//			num->value = -num->value;
//			delete negative;
//		}
//	}
//
//	if (GetOperateNum(now->eOperator) == 2)
//	{
//		//下列每种情况必须互斥，因为每个情况都有返回值，涉及删除操作，若不返回连续执行将导致指针出错
//		//不检查左右儿子是否存在，因为此处本身就是2元运算符
//		bool LChildIs0 = (now->left->eType == NODE_NUMBER && std::abs(now->left->value) < MIN_DOUBLE);
//		bool RChildIs0 = (now->right->eType == NODE_NUMBER && std::abs(now->right->value) < MIN_DOUBLE);
//		bool LChildIs1 = (now->left->eType == NODE_NUMBER && std::abs(now->left->value - 1) < MIN_DOUBLE);
//		bool RChildIs1 = (now->right->eType == NODE_NUMBER && std::abs(now->right->value - 1) < MIN_DOUBLE);
//
//		//被除数为0
//		if (now->eOperator == MATH_DIVIDE && RChildIs0)
//			throw MathError{ ErrorType::ERROR_DIVIDE_ZERO, "" };
//
//		//0的0次方
//		if (now->eOperator == MATH_POWER && LChildIs0 && RChildIs0)
//			throw MathError{ ErrorType::ERROR_ZERO_POWEROF_ZERO, "" };
//
//		//若左右儿子都是数字，则计算出来
//		if (now->left->eType == NODE_NUMBER && now->right->eType == NODE_NUMBER)
//		{
//			//try
//			//{
//			CalcNode(now, now->left, now->right);
//			delete now->left;
//			delete now->right;
//			now->eOperator = MATH_NULL;
//			now->left = NULL;
//			now->right = NULL;
//			//}
//			//catch (enumError err)
//			//{
//			//	return nowError = err;
//			//}
//		}
//
//		//任何节点的0次方均等于1，除了0的0次方已在前面报错
//		if (now->eOperator == MATH_POWER && RChildIs0)
//		{
//			//替换掉当前运算符，这个1节点将在回溯时处理
//			//新建一个1节点
//			TNode *temp;
//			temp = new TNode;
//			temp->eType = NODE_NUMBER;
//			temp->value = 1;
//
//			//0节点连接到上面
//			if (now != head)
//			{
//				if (now->parent->left == now)
//				{
//					now->parent->left = temp;
//					temp->parent = now->parent;
//				}
//				if (now->parent->right == now)
//				{
//					now->parent->right = temp;
//					temp->parent = now->parent;
//				}
//			}
//			else
//				head = temp;
//
//			DeleteNode(now);
//		}
//
//		//任何数乘或被乘0、被0除、0的除0外的任何次方，等于0
//		if ((now->eOperator == MATH_MULTIPLY && (LChildIs0 || RChildIs0)) ||
//			(now->eOperator == MATH_DIVIDE && LChildIs0) ||
//			(now->eOperator == MATH_POWER && LChildIs0))
//		{
//			//替换掉当前运算符，这个0节点将在回溯时处理
//			//新建一个0节点
//			TNode *temp;
//			temp = new TNode;
//			temp->eType = NODE_NUMBER;
//			temp->value = 0;
//
//			//0节点连接到上面
//			if (now != head)
//			{
//				if (now->parent->left == now)
//				{
//					now->parent->left = temp;
//					temp->parent = now->parent;
//				}
//				if (now->parent->right == now)
//				{
//					now->parent->right = temp;
//					temp->parent = now->parent;
//				}
//			}
//			else
//				head = temp;
//
//			DeleteNode(now);
//		}
//
//		//0-x=-x
//		if (now->eOperator == MATH_SUB && LChildIs0)
//		{
//			TNode *LChild = now->left;
//			TNode *RChild = now->right;
//			now->eOperator = MATH_NEGATIVE;
//			now->left = RChild;
//			now->right = NULL;
//
//			delete LChild;
//		}
//
//		//任何数加或被加0、被减0、乘或被乘1、被1除、开1次方，等于自身
//		if (
//			(now->eOperator == MATH_ADD && (LChildIs0 || RChildIs0)) ||
//			(now->eOperator == MATH_SUB && RChildIs0) ||
//			(now->eOperator == MATH_MULTIPLY && (LChildIs1 || RChildIs1)) ||
//
//			(now->eOperator == MATH_DIVIDE && RChildIs1) ||
//			(now->eOperator == MATH_POWER && RChildIs1))
//		{
//			TNode *remain = NULL, *num = NULL;
//			if (LChildIs1 || LChildIs0)
//			{
//				num = now->left;
//				remain = now->right;
//			}
//			if (RChildIs1 || RChildIs0)
//			{
//				num = now->right;
//				remain = now->left;
//			}
//
//			//连接父级和剩余项
//			if (now != head)
//			{
//				if (now->parent->left == now)
//				{
//					now->parent->left = remain;
//					remain->parent = now->parent;
//				}
//				if (now->parent->right == now)
//				{
//					now->parent->right = remain;
//					remain->parent = now->parent;
//				}
//			}
//			else
//			{
//				head = remain;
//				head->parent = NULL;
//			}
//			delete num;
//			delete now;
//		}
//
//
//	}
//
//}
//
//
//
// void TExpressionTree::Solve(TNode *now, TNode *&write_pos)
//{
//	TNode *parent = now->parent;
//	if (parent != NULL)
//	{
//		TNode *brother;
//		bool bVarIsLeft;
//		if (parent->left == now)
//		{
//			brother = parent->right;
//			bVarIsLeft = true;
//		}
//		else
//		{
//			brother = parent->left;
//			bVarIsLeft = false;
//		}
//
//		switch (parent->eOperator)
//		{
//		case MATH_ADD:
//			write_pos->eType = NODE_OPERATOR;
//			write_pos->eOperator = MATH_SUB;
//
//			write_pos->right = CopyNodeTree(brother);
//			write_pos->right->parent = write_pos;
//
//			write_pos->left = new TNode;
//
//			write_pos->left->parent = write_pos;
//			Solve(parent, write_pos->left);
//			break;
//		case MATH_MULTIPLY:
//			write_pos->eType = NODE_OPERATOR;
//			write_pos->eOperator = MATH_DIVIDE;
//
//			write_pos->right = CopyNodeTree(brother);
//			write_pos->right->parent = write_pos;
//
//			write_pos->left = new TNode;
//
//			write_pos->left->parent = write_pos;
//			Solve(parent, write_pos->left);
//			break;
//		case MATH_SUB://分左右
//			if (bVarIsLeft)//被减数
//			{
//				write_pos->eType = NODE_OPERATOR;
//				write_pos->eOperator = MATH_ADD;
//
//				write_pos->right = CopyNodeTree(brother);
//				write_pos->right->parent = write_pos;
//
//				write_pos->left = new TNode;
//
//				write_pos->left->parent = write_pos;
//				Solve(parent, write_pos->left);
//			}
//			else
//			{
//				write_pos->eType = NODE_OPERATOR;
//				write_pos->eOperator = MATH_SUB;
//
//				write_pos->left = CopyNodeTree(brother);
//				write_pos->left->parent = write_pos;
//
//				write_pos->right = new TNode;
//
//				write_pos->right->parent = write_pos;
//				Solve(parent, write_pos->right);
//			}
//			break;
//		case MATH_DIVIDE://分左右
//			if (bVarIsLeft)//被除数
//			{
//				write_pos->eType = NODE_OPERATOR;
//				write_pos->eOperator = MATH_MULTIPLY;
//
//				write_pos->right = CopyNodeTree(brother);
//				write_pos->right->parent = write_pos;
//
//				write_pos->left = new TNode;
//
//				write_pos->left->parent = write_pos;
//				Solve(parent, write_pos->left);
//			}
//			else
//			{
//				write_pos->eType = NODE_OPERATOR;
//				write_pos->eOperator = MATH_DIVIDE;
//
//				write_pos->left = CopyNodeTree(brother);
//				write_pos->left->parent = write_pos;
//
//				write_pos->right = new TNode;
//
//				write_pos->right->parent = write_pos;
//				Solve(parent, write_pos->right);
//			}
//			break;
//		case MATH_POSITIVE:
//			write_pos->eType = NODE_OPERATOR;
//			write_pos->eOperator = MATH_POSITIVE;
//
//			write_pos->left = new TNode;
//
//			write_pos->left->parent = write_pos;
//			Solve(parent, write_pos->left);
//			break;
//		case MATH_NEGATIVE:
//			write_pos->eType = NODE_OPERATOR;
//			write_pos->eOperator = MATH_NEGATIVE;
//
//			write_pos->left = new TNode;
//
//			write_pos->left->parent = write_pos;
//			Solve(parent, write_pos->left);
//			break;
//		case MATH_SIN:
//			write_pos->eType = NODE_FUNCTION;
//			write_pos->eOperator = MATH_ARCSIN;
//
//			write_pos->left = new TNode;
//
//			write_pos->left->parent = write_pos;
//			Solve(parent, write_pos->left);
//			break;
//		case MATH_COS:
//			write_pos->eType = NODE_FUNCTION;
//			write_pos->eOperator = MATH_ARCCOS;
//
//			write_pos->left = new TNode;
//
//			write_pos->left->parent = write_pos;
//			Solve(parent, write_pos->left);
//			break;
//		case MATH_TAN:
//			write_pos->eType = NODE_FUNCTION;
//			write_pos->eOperator = MATH_ARCTAN;
//
//			write_pos->left = new TNode;
//
//			write_pos->left->parent = write_pos;
//			Solve(parent, write_pos->left);
//			break;
//		default:
//			assert(0);
//			break;
//		}
//	}
//	else
//	{
//		//have not parent
//		write_pos->eType = NODE_NUMBER;
//		write_pos->value = 0.0;
//
//	}
//}
//
////求解单变量方程 不验证可求解性，需提前调用HasOnlyOneVar确认
// std::string  TExpressionTree::Solve(std::string &var, double &value)
//{
//	TExpressionTree Result;
//
//	TNode *ResultNow = new TNode;
//
//	var = LastVarNode->varname;
//
//	Solve(LastVarNode, ResultNow);
//
//	Result.head = ResultNow;
//
//	value = Result.Value(true);
//
//	return OutputStr();
//
//}
//
//
//}

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
    explicit Mat(int row, int col) noexcept;

    explicit Mat(int row, int col, double initValue) noexcept;

    Mat(const std::vector<std::vector<double>> &init) noexcept;
    Mat(std::vector<std::vector<double>> &&init) noexcept;

    Mat(const std::initializer_list<std::initializer_list<double>> &init) noexcept;

    std::vector<double> &operator[](std::size_t i) noexcept;

    const std::vector<double> &operator[](std::size_t i) const noexcept;

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

    std::string ToString() const noexcept;

    void Resize(int newRows) noexcept;

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
    std::vector<std::vector<double>> data;

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
    explicit Vec(int rows) noexcept;

    explicit Vec(int rows, double initValue) noexcept;

    Vec(const std::initializer_list<double> &init) noexcept;

    Mat &AsMat() noexcept;

    void Resize(int newRows) noexcept;

    double &operator[](std::size_t i) noexcept {
        return data[i][0];
    }

    double operator[](std::size_t i) const noexcept {
        return data[i][0];
    }

    Vec operator+(const Vec &b) const noexcept {
        assert(rows == b.rows);
        assert(cols == 1 && b.cols == 1);
        Vec ans(b);
        for (int i = 0; i < rows; ++i) {
            ans[i] = data[i][0] + b[i];
        }
        return ans;
    }

    // be negative
    Vec operator-() noexcept {
        Vec ans(*this);
        for (auto &vec : ans.data)
            vec[0] = -vec[0];
        return ans;
    }

    Vec operator-(const Vec &b) const noexcept {
        assert(rows == b.rows);
        Vec ans(b);
        for (int i = 0; i < rows; ++i) {
            ans[i] = data[i][0] - b[i];
        }
        return ans;
    }

    Vec operator*(double m) const noexcept {
        Vec ans = *this;
        for (auto &vec : ans.data)
            vec[0] *= m;
        return ans;
    }

    Vec operator*(const Vec &b) const noexcept {
        assert(rows == b.rows);
        Vec ans(b);
        for (int i = 0; i < rows; ++i) {
            ans[i] = data[i][0] * b[i];
        }
        return ans;
    }

    Vec operator/(const Vec &b) const noexcept {
        assert(rows == b.rows);
        Vec ans(b);
        for (int i = 0; i < rows; ++i) {
            ans[i] = data[i][0] / b[i];
        }
        return ans;
    }

    bool operator<(const Vec &b) noexcept {
        assert(rows == b.rows);
        for (int i = 0; i < rows; ++i)
            if (data[i][0] >= b[i])
                return false;
        return true;
    }

    friend double Dot(const Vec &a, const Vec &b) noexcept;
    friend Vec operator*(double k, const Vec &V);
};

/**
 * 向量点乘。
 */
double Dot(const Vec &a, const Vec &b) noexcept;

} // namespace tomsolver

namespace tomsolver {

Mat::Mat(int row, int col) noexcept
    : rows(row), cols(col), data(std::vector<std::vector<double>>(row, std::vector<double>(col))) {
    assert(row > 0);
    assert(col > 0);
}

Mat::Mat(int row, int col, double initValue) noexcept
    : rows(row), cols(col), data(std::vector<std::vector<double>>(row, std::vector<double>(col, initValue))) {}

Mat::Mat(const std::vector<std::vector<double>> &init) noexcept {
    rows = static_cast<int>(init.size());
    assert(rows > 0);
    cols = static_cast<int>(init[0].size());
    assert(cols > 0);
#ifndef NDEBUG
    for (auto &vec : init) {
        assert(static_cast<int>(vec.size()) == cols);
    }
#endif
    data = init;
}

Mat::Mat(std::vector<std::vector<double>> &&init) noexcept
    : rows(static_cast<int>(init.size())), cols(static_cast<int>(init.size() ? init[0].size() : 0)),
      data(std::move(init)) {}

Mat::Mat(const std::initializer_list<std::initializer_list<double>> &init) noexcept {
    rows = static_cast<int>(init.size());
    assert(rows > 0);
    cols = static_cast<int>(init.begin()->size());
    assert(cols > 0);
#ifndef NDEBUG
    for (auto &vec : init) {
        assert(static_cast<int>(vec.size()) == cols);
    }
#endif
    data = std::vector<std::vector<double>>(init.begin(), init.end());
}

std::vector<double> &Mat::operator[](std::size_t i) noexcept {
    return data[i];
}

const std::vector<double> &Mat::operator[](std::size_t i) const noexcept {
    return data[i];
}

bool Mat::operator==(double m) const noexcept {
    for (auto &vec : data)
        for (auto &val : vec) {
            if (std::abs(val - m) >= GetConfig().epsilon)
                return false;
        }
    return true;
}

bool Mat::operator==(const Mat &b) const noexcept {
    assert(rows == b.rows);
    assert(cols == b.cols);
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            if (std::abs(data[i][j] - b[i][j]) > GetConfig().epsilon)
                return false;
    return true;
}

// be negative
Mat Mat::operator-() noexcept {
    Mat ans(*this);
    for (auto &vec : ans.data)
        for (auto &val : vec)
            val = -val;
    return ans;
}

Mat Mat::operator+(const Mat &b) const noexcept {
    assert(rows == b.rows);
    assert(cols == b.cols);
    Mat ans(b);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            ans[i][j] = data[i][j] + b[i][j];
        }
    }
    return ans;
}

Mat &Mat::operator+=(const Mat &b) noexcept {
    assert(rows == b.rows);
    assert(cols == b.cols);
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            data[i][j] += b[i][j];
    return *this;
}

Mat Mat::operator-(const Mat &b) const noexcept {
    assert(rows == b.rows);
    assert(cols == b.cols);
    Mat ans(b);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            ans[i][j] = data[i][j] - b[i][j];
        }
    }
    return ans;
}

Mat Mat::operator*(double m) const noexcept {
    Mat ans = *this;
    for (auto &vec : ans.data)
        for (auto &val : vec)
            val *= m;
    return ans;
}

Mat Mat::operator*(const Mat &b) const noexcept {
    assert(cols == b.rows);
    Mat ans(rows, b.cols);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < b.cols; ++j) {
            double sum = 0;
            for (int k = 0; k < cols; ++k) {
                sum += data[i][k] * b[k][j];
            }
            ans[i][j] = sum;
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
    if (i == j)
        return *this;
    assert(i >= 0);
    assert(i < rows);
    assert(j >= 0);
    assert(j < rows);

    std::swap(data[i], data[j]);
    return *this;
}

std::string Mat::ToString() const noexcept {
    if (data.empty())
        return "[]";
    std::string s;
    s.reserve(256);

    auto OutputRow = [&](int i) {
        int j = 0;
        for (; j < Cols() - 1; ++j) {
            s += tomsolver::ToString(data[i][j]) + ", ";
        }
        s += tomsolver::ToString(data[i][j]);
    };

    s += "[";
    OutputRow(0);
    s += "\n";

    int i = 1;
    for (; i < Rows() - 1; ++i) {
        s += " ";
        OutputRow(i);
        s += "\n";
    }
    s += " ";
    OutputRow(i);
    s += "]";
    return s;
}

void Mat::Resize(int newRows) noexcept {
    assert(newRows > 0);
    if (newRows < rows)
        data.resize(newRows);
    else {
        data.resize(newRows);
        for (int i = rows; i < newRows; ++i)
            data[i] = std::vector<double>(cols, 0);
    }
    rows = newRows;
}

Mat &Mat::Zero() noexcept {
    data = std::vector<std::vector<double>>(rows, std::vector<double>(cols, 0));
    return *this;
}

Mat &Mat::Ones() noexcept {
    assert(rows == cols);
    Zero();
    for (int i = 0; i < rows; ++i)
        data[i][i] = 1;
    return *this;
}

double Mat::Norm2() const noexcept {
    double sum = 0;
    for (auto &vec : data)
        for (auto val : vec)
            sum += val * val;
    return sum;
}

double Mat::NormInfinity() const noexcept {
    double ans = data[0][0];
    for (auto &vec : data)
        for (auto val : vec)
            ans = std::max(ans, std::abs(val));
    return ans;
}

double Mat::NormNegInfinity() const noexcept {
    double ans = data[0][0];
    for (auto &vec : data)
        for (auto val : vec)
            ans = std::min(ans, std::abs(val));
    return ans;
}

double Mat::Min() const noexcept {
    double ans = data[0][0];
    for (auto &vec : data)
        for (auto val : vec)
            ans = std::min(ans, val);
    return ans;
}

void Mat::SetValue(double value) noexcept {
    data = std::vector<std::vector<double>>(rows, std::vector<double>(cols, value));
}

bool Mat::PositiveDetermine() const noexcept {
    assert(rows == cols);
    for (int i = 1; i <= rows; ++i) {
        double det = Det(*this, i);
        if (det <= 0)
            return false;
    }
    return true;
}

Mat Mat::Transpose() const noexcept {
    Mat ans(cols, rows);
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j) {
            ans[j][i] = data[i][j];
        }
    return ans;
}

Mat Mat::Inverse() const {
    assert(rows == cols);
    const Mat &A = *this;
    int n = rows;
    Mat ans(n, n);
    double det = Det(A, n); // Determinant, 역행렬을 시킬 행렬의 행렬식을 구함

    if (std::abs(det) <= GetConfig().epsilon) // 0일때는 예외처리 (역행렬을 구할 수 없기 때문.)
    {
        throw MathError(ErrorType::ERROR_SINGULAR_MATRIX, "");
    }

    Mat adj(n, n); // 딸림행렬 선언

    Adjoint(A, adj); // 딸림행렬 초기화

    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            ans[i][j] = adj[i][j] / det; // 각 행렬의 원소들을 조합해 값을 도출한다.
    // 역행렬의 공식 -> 역행렬 = 1 / 행렬식 * 딸림행렬
    // 역행렬[i][j]번째 원소 = 딸림행렬[i][j]번째 원소 / 행렬식

    return ans;
}

Mat operator*(double k, const Mat &mat) noexcept {
    Mat ans(mat);
    for (int i = 0; i < ans.rows; ++i)
        for (int j = 0; j < ans.cols; ++j)
            ans[i][j] *= k;
    return ans;
}

Mat EachDivide(const Mat &a, const Mat &b) noexcept {
    assert(a.rows == b.rows);
    assert(a.cols == b.cols);
    Mat ans(a);
    for (int i = 0; i < a.rows; ++i)
        for (int j = 0; j < a.cols; ++j) {
            ans[i][j] /= b[i][j];
        }
    return ans;
}

bool IsZero(const Mat &mat) noexcept {
    for (auto &vec : mat.data)
        for (auto d : vec)
            if (d > GetConfig().epsilon)
                return false;
    return true;
}

bool AllIsLessThan(const Mat &v1, const Mat &v2) noexcept {
    assert(v1.rows == v2.rows && v1.cols == v2.cols);
    for (int i = 0; i < v1.rows; ++i) {
        for (int j = 0; j < v1.cols; ++j) {
            if (v1[i][j] > v2[i][j])
                return false;
        }
    }
    return true;
}

int GetMaxAbsRowIndex(const Mat &A, int rowStart, int rowEnd, int col) noexcept {
    double max = 0.0;
    int index = rowStart;
    for (int i = rowStart; i <= rowEnd; i++) {
        if (std::abs(A[i][col]) > max) {
            max = std::abs(A[i][col]);
            index = i;
        }
    }
    return index;
}

void Adjoint(const Mat &A, Mat &adj) noexcept // 딸림행렬, 수반행렬
{
    if (A.rows == 1) // 예외처리
    {
        adj[0][0] = 1;
        return;
    }

    int sign = 1;

    Mat temp(A.rows, A.cols);

    for (int i = 0; i < A.rows; i++) {
        for (int j = 0; j < A.cols; j++) {
            GetCofactor(A, temp, i, j, A.rows); // 여인수 구하기, 단 i, j값으로 되기에 temp는 항상 바뀐다.

            sign = ((i + j) % 2 == 0) ? 1 : -1; // +, -, + 형식으로 되는데, 0,0 좌표면 +, 0,1좌표면 -, 이렇게 된다.

            adj[j][i] = (sign) * (Det(temp, A.rows - 1)); // n - 1 X n - 1 은, 언제나 각 여인수 행렬 은 여인수를
                                                          // 따오는 행렬의 크기 - 1 이기 때문이다.
        }
    }
}

void GetCofactor(const Mat &A, Mat &temp, int p, int q,
                 int n) noexcept // 여인수를 구해다주는 함수!
{
    int i = 0, j = 0; // n - 1 X n - 1 행렬에 넣을 x, y좌표

    for (int row = 0; row < n; row++) {
        for (int col = 0; col < n; col++) {
            if (row != p && col != q) // 하나라도 안되면 안들어감! && 이기때문
            {
                temp[i][j++] = A[row][col]; // j++ <- 후처리로 따로 풀어보면, [i][j] 한 뒤, j++한거와 똑같음

                if (j == n - 1) // 1차원 작게, 3차원이면 2차원 여인수 모음 행렬이 나오니까
                {
                    j = 0; // x좌표 초기화
                    i++;   // y좌표 ++
                }
            }
        }
    }
}

double Det(const Mat &A, int n) noexcept {
    if (n == 0)
        return 0;

    if (n == 1)
        return A[0][0];

    if (n == 2) // 계산 압축
        return ((A[0][0] * A[1][1]) - (A[1][0] * A[0][1]));

    Mat temp(A.rows - 1, A.cols - 1); // n X n 행렬의 여인수를 담을 임시 행렬

    double D = 0; // D = 한 행렬의 Determinant값

    int sign = 1; // sign = +, -, +, -.... 형태로 지속되는 결과값에 영향을 주는 정수

    for (int f = 0; f < n; f++) {
        GetCofactor(A, temp, 0, f, n); // 0으로 고정시킨 이유는, 수학 공식 상 Determinant (행렬식)은 n개의 열 중
                                       // 아무거나 잡아도 결과값은 모두 일치하기 때문
        D += sign * A[0][f] * Det(temp, n - 1); // 재귀 형식으로 돌아간다. f는 n X n 중 정수 n을 향해 간다.

        sign = -sign; // +, -, +, -... 형식으로 되기 때문에 반대로 만들어준다.
    }

    return D; // 마지막엔 n X n 행렬의 Determinant를 리턴해준다.
}

Vec::Vec(int rows) noexcept : Mat(rows, 1) {}

Vec::Vec(int rows, double initValue) noexcept : Mat(rows, 1, initValue) {}

Vec::Vec(const std::initializer_list<double> &init) noexcept : Vec(static_cast<int>(init.size())) {
    data.resize(rows, std::vector<double>(1));
    int i = 0;
    for (auto v : init)
        data[i++][0] = v;
}

Mat &Vec::AsMat() noexcept {
    return *this;
}

void Vec::Resize(int newRows) noexcept {
    assert(newRows > 0);
    data.resize(newRows, std::vector<double>(1));
    rows = newRows;
}

double Dot(const Vec &a, const Vec &b) noexcept {
    assert(a.rows == b.rows);
    int n = a.rows;
    double sum = 0;
    for (int i = 0; i < n; ++i)
        sum += a[i] * b[i];
    return sum;
}

Vec operator*(double k, const Vec &v) {
    Vec ans(v);
    for (int i = 0; i < ans.rows; ++i)
        ans[i] *= k;
    return ans;
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
 * @exception MathError 不定方程（设置GetConfig().allowIndeterminateEquation=true可以允许不定方程组返回一组特解）
 *
 */
Vec SolveLinear(const Mat &A, const Vec &b);

} // namespace tomsolver

namespace tomsolver {

void SwapRow(Mat &A, Vec &b, int i, int j) {
    if (i == j)
        return;
    std::swap(A[i], A[j]);
    std::swap(b[i], b[j]);
}

Vec SolveLinear(const Mat &AA, const Vec &bb) {
    Mat A(AA);
    Vec b(bb);

    int rows = A.Rows(); // 行数
    int cols = rows;     // 列数=未知数个数

    int RankA = rows, RankAb = rows; // 初始值

    assert(rows == b.Rows()); // A行数不等于b行数

    Vec ret(rows);

    if (rows > 0)
        cols = static_cast<int>(A[0].size());
    if (cols != rows) // 不是方阵
    {
        if (rows > cols)
            throw MathError(ErrorType::ERROR_OVER_DETERMINED_EQUATIONS, ""); // 过定义方程组
        else                                                                 // 不定方程组
            ret.Resize(cols);
    }

    std::vector<decltype(rows)> TrueRowNumber(cols);

    // 列主元消元法
    for (decltype(rows) y = 0, x = 0; y < rows && x < cols; y++, x++) {
        // if (A[i].size() != rows)

        // 从当前行(y)到最后一行(rows-1)中，找出x列最大的一行与y行交换
        int maxAbsRowIndex = GetMaxAbsRowIndex(A, y, rows - 1, x);
        A.SwapRow(y, maxAbsRowIndex);
        b.SwapRow(y, maxAbsRowIndex);

        while (std::abs(A[y][x]) < GetConfig().epsilon) // 如果当前值为0  x一直递增到非0
        {
            x++;
            if (x == cols)
                break;

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
            if (std::abs(b[y]) < GetConfig().epsilon)
                RankAb = y;

            if (RankA != RankAb) // 奇异，且系数矩阵及增广矩阵秩不相等->无解
                throw MathError(ErrorType::ERROR_SINGULAR_MATRIX, "");
            else
                break; // 跳出for，得到特解
        }

        // 主对角线化为1
        double m_num = A[y][x];
        for (decltype(rows) j = y; j < cols; j++) // y行第j个->第cols个
            A[y][j] /= m_num;
        b[y] /= m_num;

        // 每行化为0
        for (decltype(rows) row = y + 1; row < rows; row++) // 下1行->最后1行
        {
            if (std::abs(A[row][x]) < GetConfig().epsilon)
                ;
            else {
                double mi = A[row][x];
                for (auto col = x; col < cols; col++) // row行第x个->第cols个
                {
                    A[row][col] -= A[y][col] * mi;
                }
                b[row] -= b[y] * mi;
            }
        }
    }

    bool bIndeterminateEquation = false; // 设置此变量是因为后面rows将=cols，标记以判断是否为不定方程组

    // 若为不定方程组，空缺行全填0继续运算
    if (rows != cols) {
        A.Resize(cols);                    // A改为cols行
        for (auto i = rows; i < cols; i++) // A从rows行开始每行cols个数
            A[i].resize(cols);
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
    double sum_others = 0.0;
    for (int i = rows - 1; i >= 0; i--) // 最后1行->第1行
    {
        sum_others = 0.0;
        for (decltype(rows) j = i + 1; j < rows; j++) // 本列 后的元素乘以已知x 加总
        {
            sum_others += A[i][j] * ret[j];
        }
        ret[i] = b[i] - sum_others;
    }

    if (RankA < cols && RankA == RankAb) {
        if (bIndeterminateEquation) {
            if (!GetConfig().allowIndeterminateEquation)
                throw MathError(ErrorType::ERROR_INDETERMINATE_EQUATION,
                                std::string("A = ") + AA.ToString() + "\nb = " + bb.ToString());
        } else
            throw MathError(ErrorType::ERROR_INFINITY_SOLUTIONS, "");
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
    explicit VarsTable(const std::initializer_list<std::pair<std::string, double>> &initList);

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
        table[var] = initValue;
    }
    assert(vars.size() == table.size() && "vars is not unique");
}

VarsTable::VarsTable(const std::initializer_list<std::pair<std::string, double>> &initList)
    : vars(initList.size()), values(static_cast<int>(initList.size())), table(initList.begin(), initList.end()) {
    int i = 0;
    for (auto &pr : initList) {
        vars[i] = pr.first;
        values[i] = pr.second;
        ++i;
    }
    assert(vars.size() == table.size() && "vars is not unique");
}

VarsTable::VarsTable(const std::map<std::string, double> &table) noexcept
    : vars(table.size()), values(static_cast<int>(table.size())), table(table) {
    int i = 0;
    for (auto &pr : table) {
        vars[i] = pr.first;
        values[i] = pr.second;
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
    if (values.Rows() != rhs.values.Rows()) {
        return false;
    }

    for (auto &pr : table) {
        const std::string &varname = pr.first;
        auto it = rhs.table.find(varname);
        if (it == rhs.table.end()) {
            return false;
        }
        double value = pr.second;
        if (std::abs(it->second - value) > GetConfig().epsilon) {
            return false;
        }
    }
    return true;
}

double VarsTable::operator[](const std::string &varname) const {
    auto it = table.find(varname);
    if (it == table.end()) {
        throw std::out_of_range("no such variable: " + varname);
    }
    return it->second;
}

std::ostream &operator<<(std::ostream &out, const VarsTable &table) noexcept {
    for (auto &pr : table) {
        out << pr.first << " = " << tomsolver::ToString(pr.second) << std::endl;
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

    bool Equal(const std::unique_ptr<NodeImpl> &rhs) const noexcept;

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
     * 化简。
     */
    NodeImpl &Simplify() noexcept;

    /**
     * 返回表达式内出现的所有变量名。
     */
    std::set<std::string> GetAllVarNames() const noexcept;

    /**
     * 检查整个节点数的parent指针是否正确。
     */
    void CheckParent() const noexcept;

private:
    NodeType type;
    MathOperator op;
    double value;
    std::string varname;
    const NodeImpl *parent;
    std::unique_ptr<NodeImpl> left, right;

    /**
     * 本节点如果是OPERATOR，检查操作数数量和left, right指针是否匹配。
     */
    void CheckOperatorNum() const noexcept;

    /**
     * 节点转string。仅限本节点，不含子节点。
     */
    std::string NodeToStr() const noexcept;

    void ToStringRecursively(std::string &output) const noexcept;

    void ToStringNonRecursively(std::string &output) const noexcept;

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

    friend std::unique_ptr<internal::NodeImpl> Operator(MathOperator op, Node &&left, Node &&right) noexcept;

    friend std::unique_ptr<NodeImpl> CloneRecursively(const std::unique_ptr<NodeImpl> &rhs) noexcept;
    friend std::unique_ptr<NodeImpl> CloneNonRecursively(const std::unique_ptr<NodeImpl> &rhs) noexcept;

    friend void CopyOrMoveTo(NodeImpl *parent, std::unique_ptr<NodeImpl> &child,
                             std::unique_ptr<NodeImpl> &&n1) noexcept;
    friend void CopyOrMoveTo(NodeImpl *parent, std::unique_ptr<NodeImpl> &child,
                             const std::unique_ptr<NodeImpl> &n1) noexcept;

    friend std::ostream &operator<<(std::ostream &out, const std::unique_ptr<NodeImpl> &n) noexcept;

    template <typename T>
    friend std::unique_ptr<NodeImpl> UnaryOperator(MathOperator op, T &&n) noexcept;

    template <typename T1, typename T2>
    friend std::unique_ptr<NodeImpl> BinaryOperator(MathOperator op, T1 &&n1, T2 &&n2) noexcept;

    friend class tomsolver::SymMat;
    friend class DiffFunctions;
    friend class SubsFunctions;
    friend class ParseFunctions;
};

std::unique_ptr<NodeImpl> CloneRecursively(const std::unique_ptr<NodeImpl> &rhs) noexcept;

std::unique_ptr<NodeImpl> CloneNonRecursively(const std::unique_ptr<NodeImpl> &rhs) noexcept;

/**
 * 对于一个节点n和另一个节点n1，把n1移动到作为n的子节点。
 * 用法：CopyOrMoveTo(n->parent, n->left, std::forward<T>(n1));
 */
void CopyOrMoveTo(NodeImpl *parent, std::unique_ptr<NodeImpl> &child, std::unique_ptr<NodeImpl> &&n1) noexcept;

/**
 * 对于一个节点n和另一个节点n1，把n1整个拷贝一份，把拷贝的副本设为n的子节点。
 * 用法：CopyOrMoveTo(n->parent, n->left, std::forward<T>(n1));
 */
void CopyOrMoveTo(NodeImpl *parent, std::unique_ptr<NodeImpl> &child, const std::unique_ptr<NodeImpl> &n1) noexcept;

/**
 * 重载std::ostream的<<操作符以输出一个Node节点。
 */
std::ostream &operator<<(std::ostream &out, const std::unique_ptr<internal::NodeImpl> &n) noexcept;

template <typename T>
std::unique_ptr<NodeImpl> UnaryOperator(MathOperator op, T &&n) noexcept {
    auto ret = std::make_unique<NodeImpl>(NodeType::OPERATOR, op, 0, "");
    CopyOrMoveTo(ret.get(), ret->left, std::forward<T>(n));
    return ret;
}

template <typename T1, typename T2>
std::unique_ptr<NodeImpl> BinaryOperator(MathOperator op, T1 &&n1, T2 &&n2) noexcept {
    auto ret = std::make_unique<NodeImpl>(NodeType::OPERATOR, op, 0, "");
    CopyOrMoveTo(ret.get(), ret->left, std::forward<T1>(n1));
    CopyOrMoveTo(ret.get(), ret->right, std::forward<T2>(n2));
    return ret;
}

/**
 * 新建一个运算符节点。
 */
std::unique_ptr<internal::NodeImpl> Operator(MathOperator op, Node &&left = nullptr, Node &&right = nullptr) noexcept;

} // namespace internal

Node Clone(const Node &rhs) noexcept;

/**
 * 对节点进行移动。等同于std::move。
 */
Node Move(Node &rhs) noexcept;

/**
 * 新建一个数值节点。
 */
std::unique_ptr<internal::NodeImpl> Num(double num) noexcept;

/**
 * 返回变量名是否有效。（只支持英文数字或者下划线，第一个字符必须是英文或者下划线）
 */
bool VarNameIsLegal(const std::string &varname) noexcept;

/**
 * 新建一个变量节点。
 * @exception runtime_error 名字不合法
 */
std::unique_ptr<internal::NodeImpl> Var(const std::string &varname);

template <typename T1, typename T2>
std::enable_if_t<std::is_same<std::decay_t<T1>, Node>::value, std::unique_ptr<internal::NodeImpl>>
operator+(T1 &&n1, T2 &&n2) noexcept {
    return internal::BinaryOperator(MathOperator::MATH_ADD, std::forward<T1>(n1), std::forward<T2>(n2));
}

template <typename T>
std::enable_if_t<std::is_same<std::decay_t<T>, Node>::value, std::unique_ptr<internal::NodeImpl>> &
operator+=(std::unique_ptr<internal::NodeImpl> &n1, T &&n2) noexcept {
    n1 = internal::BinaryOperator(MathOperator::MATH_ADD, std::move(n1), std::forward<T>(n2));
    return n1;
}

template <typename T1, typename T2>
std::enable_if_t<std::is_same<std::decay_t<T1>, Node>::value, std::unique_ptr<internal::NodeImpl>>
operator-(T1 &&n1, T2 &&n2) noexcept {
    return internal::BinaryOperator(MathOperator::MATH_SUB, std::forward<T1>(n1), std::forward<T2>(n2));
}

template <typename T1>
std::enable_if_t<std::is_same<std::decay_t<T1>, Node>::value, std::unique_ptr<internal::NodeImpl>>
operator-(T1 &&n1) noexcept {
    return internal::UnaryOperator(MathOperator::MATH_NEGATIVE, std::forward<T1>(n1));
}

template <typename T1>
std::enable_if_t<std::is_same<std::decay_t<T1>, Node>::value, std::unique_ptr<internal::NodeImpl>>
operator+(T1 &&n1) noexcept {
    return internal::UnaryOperator(MathOperator::MATH_POSITIVE, std::forward<T1>(n1));
}

template <typename T>
std::enable_if_t<std::is_same<std::decay_t<T>, Node>::value, std::unique_ptr<internal::NodeImpl>> &
operator-=(std::unique_ptr<internal::NodeImpl> &n1, T &&n2) noexcept {
    n1 = internal::BinaryOperator(MathOperator::MATH_SUB, std::move(n1), std::forward<T>(n2));
    return n1;
}

template <typename T1, typename T2>
std::enable_if_t<std::is_same<std::decay_t<T1>, Node>::value, std::unique_ptr<internal::NodeImpl>>
operator*(T1 &&n1, T2 &&n2) noexcept {
    return internal::BinaryOperator(MathOperator::MATH_MULTIPLY, std::forward<T1>(n1), std::forward<T2>(n2));
}

template <typename T>
std::enable_if_t<std::is_same<std::decay_t<T>, Node>::value, std::unique_ptr<internal::NodeImpl>> &
operator*=(std::unique_ptr<internal::NodeImpl> &n1, T &&n2) noexcept {
    n1 = internal::BinaryOperator(MathOperator::MATH_MULTIPLY, std::move(n1), std::forward<T>(n2));
    return n1;
}

template <typename T1, typename T2>
std::enable_if_t<std::is_same<std::decay_t<T1>, Node>::value, std::unique_ptr<internal::NodeImpl>>
operator/(T1 &&n1, T2 &&n2) noexcept {
    return internal::BinaryOperator(MathOperator::MATH_DIVIDE, std::forward<T1>(n1), std::forward<T2>(n2));
}

template <typename T>
std::enable_if_t<std::is_same<std::decay_t<T>, Node>::value, std::unique_ptr<internal::NodeImpl>> &
operator/=(std::unique_ptr<internal::NodeImpl> &n1, T &&n2) noexcept {
    n1 = internal::BinaryOperator(MathOperator::MATH_DIVIDE, std::move(n1), std::forward<T>(n2));
    return n1;
}

template <typename T1, typename T2>
std::enable_if_t<std::is_same<std::decay_t<T1>, Node>::value, std::unique_ptr<internal::NodeImpl>>
operator^(T1 &&n1, T2 &&n2) noexcept {
    return internal::BinaryOperator(MathOperator::MATH_POWER, std::forward<T1>(n1), std::forward<T2>(n2));
}

template <typename T>
std::enable_if_t<std::is_same<std::decay_t<T>, Node>::value, std::unique_ptr<internal::NodeImpl>> &
operator^=(std::unique_ptr<internal::NodeImpl> &n1, T &&n2) noexcept {
    n1 = internal::BinaryOperator(MathOperator::MATH_POWER, std::move(n1), std::forward<T>(n2));
    return n1;
}

} // namespace tomsolver

namespace tomsolver {

namespace internal {

NodeImpl::NodeImpl(const NodeImpl &rhs) noexcept {
    operator=(rhs);
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
    operator=(std::move(rhs));
}

NodeImpl &NodeImpl::operator=(NodeImpl &&rhs) noexcept {
    type = rhs.type;
    op = rhs.op;
    value = rhs.value;
    varname = rhs.varname;
    parent = rhs.parent;
    left = std::move(rhs.left);
    if (left)
        left->parent = this;
    right = std::move(rhs.right);
    if (right)
        right->parent = this;

    rhs.parent = nullptr;
    return *this;
}

NodeImpl::~NodeImpl() {
    Release();
}

bool NodeImpl::Equal(const std::unique_ptr<NodeImpl> &rhs) const noexcept {
    // 前序遍历。非递归实现。
    if (this == rhs.get()) {
        return true;
    }

    struct Item {
        const NodeImpl *lhs;
        const NodeImpl *rhs;
    };
    std::stack<Item> stk;

    auto IsSame = [](const Item &item) -> bool {
        return item.lhs->type == item.rhs->type && item.lhs->op == item.rhs->op && item.lhs->value == item.rhs->value &&
               item.lhs->varname == item.rhs->varname;
    };
    if (!IsSame({this, rhs.get()})) {
        return false;
    }

    if (right && rhs->right) {
        stk.push({right.get(), rhs->right.get()});
    } else {
        if (right == nullptr && rhs->right == nullptr) {
        } else {
            return false;
        }
    }
    if (left && rhs->left) {
        stk.push({left.get(), rhs->left.get()});
    } else {
        if (left == nullptr && rhs->left == nullptr) {
        } else {
            return false;
        }
    }
    while (!stk.empty()) {
        Item item = stk.top();
        stk.pop();

        // 检查
        if (!IsSame(item)) {
            return false;
        }

        if (item.lhs->right && item.rhs->right) {
            stk.push({item.lhs->right.get(), item.rhs->right.get()});
        } else {
            if (item.lhs->right == nullptr && item.rhs->right == nullptr) {
            } else {
                return false;
            }
        }
        if (item.lhs->left && item.rhs->left) {
            stk.push({item.lhs->left.get(), item.rhs->left.get()});
        } else {
            if (item.lhs->left == nullptr && item.rhs->left == nullptr) {
            } else {
                return false;
            }
        }
    }
    return true;
}

std::string NodeImpl::ToString() const noexcept {
    std::string ret;
    ToStringNonRecursively(ret);
    return ret;
}

double NodeImpl::Vpa() const {
    return VpaNonRecursively();
}

NodeImpl &NodeImpl::Calc() {
    double d = Vpa();
    type = NodeType::NUMBER;
    value = d;
    varname.clear();
    left = nullptr;
    right = nullptr;
    parent = nullptr;
    return *this;
}

void NodeImpl::CheckParent() const noexcept {
    // 前序遍历。非递归实现。

    std::stack<const NodeImpl *> stk;

    CheckOperatorNum();
    if (right) {
        stk.push(right.get());
    }
    if (left) {
        stk.push(left.get());
    }
    while (!stk.empty()) {
        const NodeImpl *f = stk.top();
        stk.pop();

#ifndef NDEBUG
        // 检查
        assert(f->parent);
        bool isLeftChild = f->parent->left.get() == f;
        bool isRightChild = f->parent->right.get() == f;
        assert(isLeftChild || isRightChild);
#endif

        f->CheckOperatorNum();

        if (f->right) {
            stk.push(f->right.get());
        }
        if (f->left) {
            stk.push(f->left.get());
        }
    }
}

void NodeImpl::CheckOperatorNum() const noexcept {
    if (type != NodeType::OPERATOR)
        return;

    if (GetOperatorNum(op) == 1) {
        assert(left);
        assert(right == nullptr);
        return;
    }
    if (GetOperatorNum(op) == 2) {
        assert(left);
        assert(right);
        return;
    }
    assert(0);
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

void NodeImpl::ToStringRecursively(std::string &output) const noexcept {
    // 中序遍历。递归实现。

    switch (type) {
    case NodeType::NUMBER:
        // 如果当前节点是数值且小于0，且前面是-运算符，那么加括号
        if (value < 0 && parent != nullptr && parent->right.get() == this && parent->op == MathOperator::MATH_SUB) {
            output += "(" + NodeToStr() + ")";
        } else {
            output += NodeToStr();
        }
        return;
    case NodeType::VARIABLE:
        output += NodeToStr();
        return;
    case NodeType::OPERATOR:
        // pass
        break;
    }

    int has_parenthesis = 0;
    if (GetOperatorNum(op) == 1) // 一元运算符：函数和取负
    {
        if (op == MathOperator::MATH_POSITIVE || op == MathOperator::MATH_NEGATIVE) {
            output += "(" + NodeToStr();
            has_parenthesis = 1;
        } else {
            output += NodeToStr() + "(";
            has_parenthesis = 1;
        }
    } else {
        // 非一元运算符才输出，即一元运算符的输出顺序已改变
        if (type == NodeType::OPERATOR) // 本级为运算符
            if (parent != nullptr)
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
                    output += "(";
                    has_parenthesis = 1;
                }
    }

    if (left != nullptr) // 左遍历
    {
        left->ToStringRecursively(output);
    }

    if (GetOperatorNum(op) != 1) // 非一元运算符才输出，即一元运算符的输出顺序已改变
    {
        output += NodeToStr();
    }

    if (right != nullptr) // 右遍历
    {
        right->ToStringRecursively(output);
    }

    // 回到本级时补齐右括号，包住前面的东西
    if (has_parenthesis) {
        output += ")";
    }
}

void NodeImpl::ToStringNonRecursively(std::string &output) const noexcept {
    // 中序遍历。非递归实现。
    std::stack<const NodeImpl *> stk;
    const NodeImpl *cur = this;

    NodeImpl rightParenthesis(NodeType::OPERATOR, MathOperator::MATH_RIGHT_PARENTHESIS, 0, "");

    auto AddLeftLine = [&](const NodeImpl *cur) {
        while (cur) {
            if (cur->type != NodeType::OPERATOR) {
                stk.push(cur);
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
                    output += cur->NodeToStr();
                    cur = cur->left.get();
                    continue;
                }
                output += cur->NodeToStr() + "(";

                // not push this op

                // push ')'
                stk.push(&rightParenthesis);

                cur = cur->left.get();
                continue;
            }

            // 二元运算符的特殊处理：
            const NodeImpl *curParent = cur->parent;
            if (curParent != nullptr) {
                if ((GetOperatorNum(curParent->op) == 2 && // 父运算符存在，为二元，
                     (Rank(curParent->op) > Rank(cur->op)  // 父级优先级高于本级->加括号

                      || ( // 两级优先级相等
                             Rank(curParent->op) == Rank(cur->op) &&
                             (
                                 // 本级为父级的右子树 且父级不满足结合律->加括号
                                 (InAssociativeLaws(curParent->op) == false && cur == curParent->right.get()) ||
                                 // 两级都是右结合
                                 (InAssociativeLaws(curParent->op) == false && IsLeft2Right(cur->op) == false)))))

                    //||

                    ////父运算符存在，为除号，且本级为分子，则添加括号
                    //(now->parent->eOperator == MATH_DIVIDE && now == now->parent->right)
                ) {
                    output += "(";

                    // push ')'
                    stk.push(&rightParenthesis);

                    stk.push(cur);
                    cur = cur->left.get();
                    continue;
                }
            }

            stk.push(cur);
            cur = cur->left.get();
        }
    };
    AddLeftLine(cur);

    while (!stk.empty()) {
        cur = stk.top();
        stk.pop();

        // output

        // 负数的特殊处理
        // 如果当前节点是数值且小于0，且前面是-运算符，那么加括号
        if (cur->type == NodeType::NUMBER && cur->value < 0 && cur->parent != nullptr &&
            cur->parent->right.get() == cur && cur->parent->op == MathOperator::MATH_SUB) {
            output += "(" + cur->NodeToStr() + ")";
        } else {
            output += cur->NodeToStr();
        }

        if (cur->right) {
            cur = cur->right.get();
            AddLeftLine(cur);
            continue;
        }
    }
}

double NodeImpl::VpaRecursively() const {
    // 后序遍历。递归实现。

    double l = 0, r = 0;
    if (left != nullptr)
        l = left->Vpa();
    if (right != nullptr)
        r = right->Vpa();

    if (type == NodeType::NUMBER) {
        return value;
    }

    if (type == NodeType::VARIABLE) {
        throw std::runtime_error("has variable. can not calculate to be a number");
    }

    if (type == NodeType::OPERATOR) {
        assert((GetOperatorNum(op) == 1 && left != nullptr && right == nullptr) ||
               (GetOperatorNum(op) == 2 && left != nullptr && right != nullptr));
        return tomsolver::Calc(op, l, r);
    }

    throw std::runtime_error("unsupported node type");
}

double NodeImpl::VpaNonRecursively() const {
    // 后序遍历。非递归实现。

    std::stack<const NodeImpl *> stk;
    std::vector<const NodeImpl *> revertedPostOrder;

    // ==== Part I ====

    // 借助一个栈，得到反向的后序遍历序列，结果保存在revertedPostOrder
    stk.push(this);

    while (1) {
        if (stk.empty()) {
            break;
        }

        auto f = stk.top();
        stk.pop();

        if (f->left) {
            stk.push(f->left.get());
        }

        if (f->right) {
            stk.push(f->right.get());
        }

        revertedPostOrder.push_back(f);
    }

    // ==== Part II ====
    // revertedPostOrder的反向序列是一组逆波兰表达式，根据这组逆波兰表达式可以计算出表达式的值
    // calcStk是用来计算值的临时栈，计算完成后calcStk的size应该为1
    std::stack<double> calcStk;
    for (auto it = revertedPostOrder.rbegin(); it != revertedPostOrder.rend(); ++it) {
        auto f = *it;

        if (f->type == NodeType::NUMBER) {
            calcStk.push(f->value);
            continue;
        }

        if (f->type == NodeType::OPERATOR) {
            if (GetOperatorNum(f->op) == 1) {
                calcStk.top() = tomsolver::Calc(f->op, calcStk.top(), std::numeric_limits<double>::quiet_NaN());
                continue;
            }

            if (GetOperatorNum(f->op) == 2) {
                double r = calcStk.top();
                calcStk.pop();

                double &l = calcStk.top();
                calcStk.top() = tomsolver::Calc(f->op, l, r);
                continue;
            }

            assert(0 && "[VpaNonRecursively] unsupported operator num");
        }

        // 其他情况抛异常
        throw std::runtime_error("wrong");
    }

    assert(calcStk.size() == 1);

    return calcStk.top();
}

void NodeImpl::Release() noexcept {
    // 后序遍历。因为要在左右儿子都没有的情况下删除节点。

    std::stack<Node> stk;

    if (left) {
        stk.push(std::move(left));
    }

    if (right) {
        stk.push(std::move(right));
    }

    while (1) {
        if (stk.empty()) {
            break;
        }

        Node f = std::move(stk.top());
        stk.pop();

        if (f->left) {
            stk.push(std::move(f->left));
        }

        if (f->right) {
            stk.push(std::move(f->right));
        }

        assert(f->left == nullptr && f->right == nullptr);

        // 这里如果把f填入vector，最后翻转。得到的序列就是后序遍历。

        // 这里f会被自动释放。
    }
}

std::unique_ptr<NodeImpl> CloneRecursively(const std::unique_ptr<NodeImpl> &rhs) noexcept {
    auto ret = std::make_unique<NodeImpl>(rhs->type, rhs->op, rhs->value, rhs->varname);
    if (rhs->left) {
        ret->left = Clone(rhs->left);
        ret->left->parent = ret.get();
    }

    if (rhs->right) {
        ret->right = Clone(rhs->right);
        ret->right->parent = ret.get();
    }
    return ret;
}

std::unique_ptr<NodeImpl> CloneNonRecursively(const std::unique_ptr<NodeImpl> &rhs) noexcept {
    // 前序遍历。非递归实现。

    struct Item {
        const NodeImpl *rhsCur;                      // 当前遍历的rhs树的节点
        const NodeImpl *parent;                      // 当前克隆节点应该连接的父节点
        std::unique_ptr<NodeImpl> &childRefOfParent; // 当前克隆节点对应的父节点的left或者right成员的引用
    };

    auto ret = std::make_unique<NodeImpl>(rhs->type, rhs->op, rhs->value, rhs->varname);
    std::stack<Item> stk;

    if (rhs->right) {
        stk.push({rhs->right.get(), ret.get(), ret->right});
    }
    if (rhs->left) {
        stk.push({rhs->left.get(), ret.get(), ret->left});
    }
    while (!stk.empty()) {
        Item item = stk.top();
        const NodeImpl *rhsCur = item.rhsCur;
        stk.pop();

        // 复制
        auto clonedNode = std::make_unique<NodeImpl>(rhsCur->type, rhsCur->op, rhsCur->value, rhsCur->varname);
        clonedNode->parent = item.parent;
        item.childRefOfParent = std::move(clonedNode);
        std::unique_ptr<NodeImpl> &cur = item.childRefOfParent;

        if (rhsCur->right) {
            stk.push({rhsCur->right.get(), cur.get(), cur->right});
        }
        if (rhsCur->left) {
            stk.push({rhsCur->left.get(), cur.get(), cur->left});
        }
    }
    return ret;
}

void CopyOrMoveTo(NodeImpl *parent, std::unique_ptr<NodeImpl> &child, std::unique_ptr<NodeImpl> &&n1) noexcept {
    n1->parent = parent;
    child = std::move(n1);
}

void CopyOrMoveTo(NodeImpl *parent, std::unique_ptr<NodeImpl> &child, const std::unique_ptr<NodeImpl> &n1) noexcept {
    auto n1Clone = std::make_unique<NodeImpl>(*n1);
    n1Clone->parent = parent;
    child = std::move(n1Clone);
}

std::ostream &operator<<(std::ostream &out, const std::unique_ptr<internal::NodeImpl> &n) noexcept {
    out << n->ToString();
    return out;
}

std::unique_ptr<internal::NodeImpl> Operator(MathOperator op, Node &&left, Node &&right) noexcept {
    auto ret = std::make_unique<internal::NodeImpl>(NodeType::OPERATOR, op, 0, "");
    if (left) {
        left->parent = ret.get();
        ret->left = std::move(left);
    }
    if (right) {
        right->parent = ret.get();
        ret->right = std::move(right);
    }
    return ret;
}

std::set<std::string> NodeImpl::GetAllVarNames() const noexcept {
    // 前序遍历。非递归实现。
    std::set<std::string> ret;

    std::stack<const NodeImpl *> stk;

    if (type == NodeType::VARIABLE) {
        ret.insert(varname);
    }

    if (right) {
        stk.push(right.get());
    }
    if (left) {
        stk.push(left.get());
    }
    while (!stk.empty()) {
        const NodeImpl *f = stk.top();
        stk.pop();

        if (f->type == NodeType::VARIABLE) {
            ret.insert(f->varname);
        }

        if (f->right) {
            stk.push(f->right.get());
        }
        if (f->left) {
            stk.push(f->left.get());
        }
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

std::unique_ptr<internal::NodeImpl> Num(double num) noexcept {
    return std::make_unique<internal::NodeImpl>(NodeType::NUMBER, MathOperator::MATH_NULL, num, "");
}

bool VarNameIsLegal(const std::string &varname) noexcept {
    if (varname.empty()) {
        return false;
    }
    char c = varname[0];
    if (!isalpha(c) && c != '_') {
        return false;
    }
    auto n = varname.size();
    for (size_t i = 1; i < n; ++i) {
        c = varname[i];
        if (isalnum(c) || c == '_') {
            continue;
        }
        return false;
    }
    return true;
}

std::unique_ptr<internal::NodeImpl> Var(const std::string &varname) {
    if (!VarNameIsLegal(varname)) {
        throw std::runtime_error("Illegal varname: " + varname);
    }
    return std::make_unique<internal::NodeImpl>(NodeType::VARIABLE, MathOperator::MATH_NULL, 0, varname);
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

Node Diff(const Node &node, const std::string &varname, int i = 1) noexcept;

Node Diff(Node &&node, const std::string &varname, int i = 1) noexcept;

} // namespace tomsolver

namespace tomsolver {

namespace internal {

class DiffFunctions {
public:
    struct DiffNode {
        NodeImpl *node;
        bool isLeftChild;

        DiffNode(NodeImpl *node, bool isLeftChild) : node(node), isLeftChild(isLeftChild) {
            if (node->parent) {
                if (node->parent->left.get() == node) {
                    if (!isLeftChild) {
                        assert(0);
                    }
                } else {
                    if (isLeftChild) {
                        assert(0);
                    }
                }
            }
        }
    };

    static void DiffOnce(std::unique_ptr<NodeImpl> &root, const std::string &varname) noexcept {
        std::queue<DiffNode> q;
        q.push(DiffNode(root.get(), true));

        while (!q.empty()) {
            DiffNode f = q.front();
            q.pop();

            switch (f.node->type) {
            case NodeType::VARIABLE:
                f.node->type = NodeType::NUMBER;
                if (f.node->varname == varname) {
                    f.node->value = 1;
                } else {
                    f.node->value = 0;
                }
                f.node->varname = "";
                break;
            case NodeType::NUMBER:
                f.node->value = 0;
                break;
            case NodeType::OPERATOR:
                DiffOnceOperator(root, f, q);
                break;
            default:
                assert(0);
            }
        }
    }

    static void DiffOnceOperator(std::unique_ptr<NodeImpl> &root, DiffNode diffNode, std::queue<DiffNode> &q) noexcept {
        NodeImpl *parent = const_cast<NodeImpl *>(diffNode.node->parent);
        auto GetUniquePtrNode = [&]() -> std::unique_ptr<NodeImpl> & {
            if (parent) {
                if (diffNode.isLeftChild) {
                    return const_cast<NodeImpl *>(diffNode.node->parent)->left;
                } else {
                    return const_cast<NodeImpl *>(diffNode.node->parent)->right;
                }
            } else {
                return root;
            }
        };
        std::unique_ptr<NodeImpl> &node = GetUniquePtrNode();

        // 调用前提：node是1元操作符
        // 如果node的成员是数字，那么整个node变为数字节点，value=0，且返回true
        // 例如： sin(1)' = 0
        auto CullNumberMember = [&]() -> bool {
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
        auto CullNumberMemberBinary = [&]() -> bool {
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

        /**
         * 应用链式法则：function(u)' = function(u) * u'
         *      函数内会生成*节点，拷贝u节点并加入求导队列，自动转移node的所有权，并连接父节点。
         * param[in/out] node为function节点
         * param[in] u为function的操作数节点
         * return 拷贝出来的u2节点的裸指针
         */
        auto ChainLaw = [&](std::unique_ptr<NodeImpl> &node, const std::unique_ptr<NodeImpl> &u) {
            Node mul = std::make_unique<NodeImpl>(NodeType::OPERATOR, MathOperator::MATH_MULTIPLY, 0, "");
            mul->parent = parent;

            node->parent = mul.get();
            mul->left = std::move(node);

            Node u2 = Clone(u);
            q.push(DiffNode(u2.get(), false));
            u2->parent = mul.get();
            mul->right = std::move(u2);

            // 连接父级
            if (parent) {
                if (diffNode.isLeftChild) {
                    parent->left = std::move(mul);
                } else {
                    parent->right = std::move(mul);
                }
            } else {
                root = std::move(mul);
            }
        };

        switch (node->op) {
        case MathOperator::MATH_NULL: {
            assert(0);
            break;
        }
        case MathOperator::MATH_POSITIVE:
        case MathOperator::MATH_NEGATIVE: {
            q.push(DiffNode(node->left.get(), true));
            return;
        }

        // 函数
        case MathOperator::MATH_SIN: {
            if (CullNumberMember()) {
                return;
            }

            // sin(u)' = cos(u) * u'

            node->op = MathOperator::MATH_COS;

            ChainLaw(node, node->left);
            break;
        }
        case MathOperator::MATH_COS: {
            if (CullNumberMember()) {
                return;
            }

            // cos(u)' = -sin(u) * u'

            Node negative = std::make_unique<NodeImpl>(NodeType::OPERATOR, MathOperator::MATH_NEGATIVE, 0, "");
            node->op = MathOperator::MATH_SIN;
            node->parent = negative.get();
            negative->left = std::move(node);

            ChainLaw(negative, negative->left->left);
            break;
        }
        case MathOperator::MATH_TAN: {
            assert(0);
            return;
        }
        case MathOperator::MATH_ARCSIN: {
            assert(0);
            return;
        }
        case MathOperator::MATH_ARCCOS: {
            assert(0);
            return;
        }
        case MathOperator::MATH_ARCTAN: {
            assert(0);
            return;
        }
        case MathOperator::MATH_SQRT: {
            assert(0);
            return;
        }
        case MathOperator::MATH_LOG: {
            if (CullNumberMember()) {
                return;
            }

            // ln(u)' = 1/u * u'
            Node &u = node->left;
            Node u2 = Clone(u);
            q.push(DiffNode(u2.get(), false));
            node = (Num(1) / Move(u)) * Move(u2);
            node->parent = parent;
            return;
        }
        case MathOperator::MATH_LOG2: {
            assert(0);
            return;
        }
        case MathOperator::MATH_LOG10: {
            assert(0);
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

            ChainLaw(node, node->left);
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
                q.push(DiffNode(node->left.get(), true));
            }
            if (node->right) {
                q.push(DiffNode(node->right.get(), false));
            }
            return;
        case MathOperator::MATH_MULTIPLY: {
            bool leftIsNumber = node->left->type == NodeType::NUMBER;
            bool rightIsNumber = node->right->type == NodeType::NUMBER;
            // 两个操作数中有一个是数字
            if (leftIsNumber) {
                q.push(DiffNode(node->right.get(), false));
                return;
            }
            if (rightIsNumber) {
                q.push(DiffNode(node->left.get(), true));
                return;
            }

            if (CullNumberMemberBinary()) {
                return;
            }

            // (u*v)' = u' * v + u * v'

            Node addNode = std::make_unique<NodeImpl>(NodeType::OPERATOR, MathOperator::MATH_ADD, 0, "");
            addNode->parent = parent;

            Node &mulLeft = node;
            Node mulRight = Clone(mulLeft);

            mulLeft->parent = addNode.get();
            addNode->left = std::move(mulLeft);

            mulRight->parent = addNode.get();
            addNode->right = std::move(mulRight);

            q.push(DiffNode(addNode->left->left.get(), true));
            q.push(DiffNode(addNode->right->right.get(), false));

            // 连接父级
            if (parent) {
                if (diffNode.isLeftChild) {
                    parent->left = std::move(addNode);
                } else {
                    parent->right = std::move(addNode);
                }
            } else {
                root = std::move(addNode);
            }
            return;
        }
        case MathOperator::MATH_DIVIDE: {
            // bool leftIsNumber = node->left->type == NodeType::NUMBER;
            bool rightIsNumber = node->right->type == NodeType::NUMBER;

            // f(x)/number = f'(x)/number
            if (rightIsNumber) {
                q.push(DiffNode(node->left.get(), true));
                return;
            }

            if (CullNumberMemberBinary()) {
                return;
            }

            // (u/v)' = (u'v - uv')/(v^2)
            Node &u = node->left;
            Node &v = node->right;
            Node u2 = Clone(u);
            Node v2 = Clone(v);
            Node v3 = Clone(v);
            q.push(DiffNode(u.get(), true));
            q.push(DiffNode(v2.get(), false));
            node = (Move(u) * Move(v) - Move(u2) * Move(v2)) / (Move(v3) ^ Num(2));
            node->parent = parent;

            return;
        }
        case MathOperator::MATH_POWER: {
            // 如果两个操作数都是数字
            if (CullNumberMemberBinary()) {
                return;
            }

            bool lChildIsNumber = node->left->type == NodeType::NUMBER;
            bool rChildIsNumber = node->right->type == NodeType::NUMBER;

            // (u^a)' = a*u^(a-1) * u'
            if (rChildIsNumber) {
                Node &a = node->right;
                double aValue = a->value;
                Node &u = node->left;
                Node u2 = Clone(u);
                q.push(DiffNode(u2.get(), false));
                node = std::move(a) * (std::move(u) ^ Num(aValue - 1)) * std::move(u2);
                node->parent = parent;
                return;
            }

            // (a^x)' = a^x * ln(a)  when a>0 and a!=1
            if (lChildIsNumber) {
                Node &a = node->left;
                double aValue = a->value;
                Node &u = node->right;
                Node u2 = Clone(u);
                q.push(DiffNode(u2.get(), false));
                node = (std::move(a) ^ std::move(u)) * log(Num(aValue)) * std::move(u2);
                node->parent = parent;
                return;
            }

            // (u^v)' = ( e^(v*ln(u)) )' = e^(v*ln(u)) * (v*ln(u))' = u^v * (v*ln(u))'
            // 左右都不是数字
            Node &u = node->left;
            Node &v = node->right;
            Node vln_u = Clone(v) * log(Clone(u));
            q.push(DiffNode(vln_u.get(), false));
            node = Move(node) * std::move(vln_u);
            node->parent = parent;

            return;
        }

        case MathOperator::MATH_AND: {
            assert(0);
            return;
        }
        case MathOperator::MATH_OR: {
            assert(0);
            return;
        }
        case MathOperator::MATH_MOD: {
            assert(0);
            return;
        }
        case MathOperator::MATH_LEFT_PARENTHESIS:
        case MathOperator::MATH_RIGHT_PARENTHESIS:
            assert(0);
            return;
        default:
            assert(0);
            return;
        }
    }
};

} // namespace internal

////未完成求导：tan,arcsin,arccos,arctan
// void TExpressionTree::Diff(TNode *now, std::string var)
//{
//	case NODE_OPERATOR://当前为运算符节点
//		switch (now->eOperator)
//		{
//	case NODE_FUNCTION:
//	{
//
//		TNode *function = now;
//		switch (function->eOperator)
//		{
//		case MATH_SQRT:
//		{
//			//转化为幂求导
//			TNode *u = function->left;
//			TNode *power = NewNode(NODE_OPERATOR, MATH_POWER);
//			TNode *num1half = NewNode(NODE_NUMBER);
//			num1half->value = 0.5;
//
//			power->left = u;
//			u->parent = power;
//
//			power->right = num1half;
//			num1half->parent = power;
//
//			if (function == head)
//				head = power;
//			else
//			{
//				if (function->parent->left == function)
//					function->parent->left = power;
//				if (function->parent->right == function)
//					function->parent->right = power;
//				power->parent = function->parent;
//			}
//
//			delete function;
//			Diff(power, var);
//
//			return;
//		}
//		case MATH_LOG:
//		case MATH_LOG10:
//		{
//			TNode *divide = NewNode(NODE_OPERATOR, MATH_DIVIDE);
//			TNode *num1 = NewNode(NODE_NUMBER);
//			num1->value = 1.0;
//
//			divide->left = num1;
//			num1->parent = divide;
//
//			TNode *u = function->left;
//
//			if (function->eOperator == MATH_LOG)//ln(x)=1/x
//			{
//				divide->right = u;
//				u->parent = divide;
//			}
//			else
//			{
//				//log10(x)=1/(x*ln(10))
//				TNode *multiply2 = NewNode(NODE_OPERATOR, MATH_MULTIPLY);
//
//				divide->right = multiply2;
//				multiply2->parent = divide;
//
//				multiply2->left = u;
//				u->parent = multiply2;
//
//				TNode *ln = NewNode(NODE_FUNCTION, MATH_LOG);
//
//				multiply2->right = ln;
//				ln->parent = multiply2;
//
//				TNode *num10 = NewNode(NODE_NUMBER);
//				num10->value = 10.0;
//
//				ln->left = num10;
//				num10->parent = ln;
//			}
//
//			TNode *top = divide;
//			TNode *u2 = NULL;
//			if (u->eType != NODE_VARIABLE)
//			{
//				TNode *multiply1 = NewNode(NODE_OPERATOR, MATH_MULTIPLY);
//				u2 = CopyNodeTree(u);
//
//				multiply1->left = divide;
//				divide->parent = multiply1;
//
//				multiply1->right = u2;
//				u2->parent = multiply1;
//
//				top = multiply1;
//			}
//
//			if (function == head)
//			{
//				head = top;
//			}
//			else
//			{
//				if (function->parent->left == function)
//				{
//					function->parent->left = top;
//					top->parent = function->parent;
//				}
//				if (function->parent->right == function)
//				{
//					function->parent->right = top;
//					top->parent = function->parent;
//				}
//			}
//			delete function;
//
//			if (u->eType != NODE_VARIABLE)
//				Diff(u2, var);
//
//		}
//		return;
//		//case MATH_ARCTAN:
//		//{
//		//	TNode *multiply = new TNode()
//		//}
//		return;
//		default:
//                    throw MathError(ErrorType::ERROR_WRONG_MATH_OPERATOR,
//                                             std::string("未完成的运算符") + Function2Str(now->eOperator));
//		}
//	}
//	}
//}

Node Diff(const Node &node, const std::string &varname, int i) noexcept {
    Node node2 = Clone(node);
    return Diff(std::move(node2), varname, i);
}

Node Diff(Node &&node, const std::string &varname, int i) noexcept {
    assert(i > 0);
    Node n = std::move(node);
    while (i--) {
        internal::DiffFunctions::DiffOnce(n, varname);
    }
#ifndef NDEBUG
    std::string s = n->ToString();
    n->CheckParent();
#endif
    n->Simplify();
#ifndef NDEBUG
    n->CheckParent();
#endif
    return n;
}

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
    SymMat(const std::initializer_list<std::initializer_list<Node>> &lst) noexcept;

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

    const std::vector<Node> &operator[](int row) const noexcept;

    std::vector<Node> &operator[](int row) noexcept;

    std::string ToString() const noexcept;

protected:
    std::vector<std::vector<Node>> data;

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
    SymVec(const std::initializer_list<Node> &lst) noexcept;

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
    static void SubsInner(Node &node, const std::map<std::string, Node> &dict) noexcept {
        // 前序遍历。非递归实现。

        std::stack<NodeImpl *> stk;

        auto Replace = [&dict](Node &cur) -> bool {
            if (cur->type != NodeType::VARIABLE) {
                return false;
            }
            const auto &itor = dict.find(cur->varname);
            if (itor == dict.end()) {
                return false;
            }

            Node cloned = Clone(itor->second);
            cloned->parent = cur->parent;
            cur = std::move(cloned);
            return true;
        };

        if (Replace(node)) {
            return;
        }

        if (node->right) {
            if (!Replace(node->right))
                stk.push(node->right.get());
        }
        if (node->left) {
            if (!Replace(node->left))
                stk.push(node->left.get());
        }
        while (!stk.empty()) {
            NodeImpl *f = stk.top();
            stk.pop();

            if (f->right) {
                if (!Replace(f->right))
                    stk.push(f->right.get());
            }
            if (f->left) {
                if (!Replace(f->left))
                    stk.push(f->left.get());
            }
        }
    }
};

} // namespace internal

Node Subs(const Node &node, const std::string &oldVar, const Node &newNode) noexcept {
    Node node2 = Clone(node);
    return Subs(std::move(node2), oldVar, newNode);
}

Node Subs(Node &&node, const std::string &oldVar, const Node &newNode) noexcept {
    Node ret = std::move(node);
    std::map<std::string, Node> dict;
    dict.insert({oldVar, Clone(newNode)});
    internal::SubsFunctions::SubsInner(ret, dict);
#ifndef NDEBUG
    ret->CheckParent();
#endif
    return ret;
}

Node Subs(const Node &node, const std::vector<std::string> &oldVars, const SymVec &newNodes) noexcept {
    Node node2 = Clone(node);
    return Subs(std::move(node2), oldVars, newNodes);
}

Node Subs(Node &&node, const std::vector<std::string> &oldVars, const SymVec &newNodes) noexcept {
    assert(static_cast<int>(oldVars.size()) == newNodes.Rows());
    Node ret = std::move(node);
    std::map<std::string, Node> dict;
    for (size_t i = 0; i < oldVars.size(); ++i) {
        dict.insert({oldVars[i], Clone(newNodes[i])});
    }
    internal::SubsFunctions::SubsInner(ret, dict);
#ifndef NDEBUG
    ret->CheckParent();
#endif
    return ret;
}

Node Subs(const Node &node, const std::map<std::string, Node> &dict) noexcept {
    Node node2 = Clone(node);
    return Subs(std::move(node2), dict);
}

Node Subs(Node &&node, const std::map<std::string, Node> &dict) noexcept {
    Node ret = std::move(node);
    internal::SubsFunctions::SubsInner(ret, dict);
#ifndef NDEBUG
    ret->CheckParent();
#endif
    return ret;
}

Node Subs(const Node &node, const std::map<std::string, double> &varValues) noexcept {
    Node node2 = Clone(node);
    return Subs(std::move(node2), varValues);
}

Node Subs(Node &&node, const std::map<std::string, double> &varValues) noexcept {
    Node ret = std::move(node);
    std::map<std::string, Node> dict;
    for (auto &pr : varValues) {
        dict.insert({pr.first, Num(pr.second)});
    }
    internal::SubsFunctions::SubsInner(ret, dict);
#ifndef NDEBUG
    ret->CheckParent();
#endif
    return ret;
}

Node Subs(const Node &node, const VarsTable &varsTable) noexcept {
    Node node2 = Clone(node);
    return Subs(std::move(node2), varsTable);
}

Node Subs(Node &&node, const VarsTable &varsTable) noexcept {
    Node ret = std::move(node);
    std::map<std::string, Node> dict;
    for (const auto &pr : varsTable) {
        dict.insert({pr.first, Num(pr.second)});
    }
    internal::SubsFunctions::SubsInner(ret, dict);
#ifndef NDEBUG
    ret->CheckParent();
#endif
    return ret;
}

} // namespace tomsolver

namespace tomsolver {

Node Simplify(const Node &node) noexcept;

Node Simplify(Node &&node) noexcept;

} // namespace tomsolver

namespace tomsolver {

namespace internal {

NodeImpl &NodeImpl::Simplify() noexcept {
    // 后序遍历。非递归实现。

    std::stack<NodeImpl *> stk;
    std::deque<NodeImpl *> revertedPostOrder;

    // ==== Part I ====

    // 借助一个栈，得到反向的后序遍历序列，结果保存在revertedPostOrder
    if (type == NodeType::OPERATOR) {
        stk.push(this);
    }

    while (1) {
        if (stk.empty()) {
            break;
        }

        auto f = stk.top();
        stk.pop();

        if (f->left && f->left->type == NodeType::OPERATOR) {
            stk.push(f->left.get());
        }

        if (f->right && f->right->type == NodeType::OPERATOR) {
            stk.push(f->right.get());
        }

        revertedPostOrder.push_back(f);
    }

    // ==== Part II ====
    std::for_each(revertedPostOrder.rbegin(), revertedPostOrder.rend(), [this](NodeImpl *&n) {
        // 对单节点n进行化简。

        // 对于1元运算符，且儿子是数字的，直接计算出来
        if (GetOperatorNum(n->op) == 1 && n->left->type == NodeType::NUMBER) {
            n->type = NodeType::NUMBER;
            n->value = tomsolver::Calc(n->op, n->left->value, 0);
            n->op = MathOperator::MATH_NULL;
            n->left = nullptr;
            return;
        }

        // 对于2元运算符
        if (GetOperatorNum(n->op) == 2) {
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
                *n = NodeImpl(NodeType::NUMBER, MathOperator::MATH_NULL, 0, "");
                return;
            }

            // 任何数加或被加0、被减0、乘或被乘1、被1除、开1次方，等于自身
            if ((n->op == MathOperator::MATH_ADD && (lChildIs0 || rChildIs0)) ||
                (n->op == MathOperator::MATH_SUB && rChildIs0) ||
                (n->op == MathOperator::MATH_MULTIPLY && (lChildIs1 || rChildIs1)) ||
                (n->op == MathOperator::MATH_DIVIDE && rChildIs1) || (n->op == MathOperator::MATH_POWER && rChildIs1)) {

                Node ret; // 要保留的节点
                if (lChildIs1 || lChildIs0) {
                    n->left = nullptr;
                    ret = std::move(n->right);
                } else if (rChildIs1 || rChildIs0) {
                    n->right = nullptr;
                    ret = std::move(n->left);
                }
                ret->parent = n->parent;

                // 连接父级和剩余项
                NodeImpl *p = const_cast<NodeImpl *>(ret->parent);
                if (p) {
                    if (p->left.get() == n) {
                        p->left = std::move(ret);
                    } else {
                        p->right = std::move(ret);
                    }
                } else {
                    *this = std::move(*ret);
                }
                return;
            }
        }
    });

    return *this;

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

} // namespace internal

Node Simplify(const Node &node) noexcept {
    Node cloned = Clone(node);
    return Simplify(std::move(cloned));
}

Node Simplify(Node &&node) noexcept {
    Node n = std::move(node);
    n->Simplify();
    return n;
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
    SingleParseError(int line, int pos, const std::string &content, const std::string &errInfo);

    virtual const char *what() const noexcept override;

    int GetLine() const noexcept;

    int GetPos() const noexcept;

private:
    int line;            // 行号
    int pos;             // 第几个字符
    std::string content; // 整行文本
    std::string errInfo; // 报错信息
    std::string whatStr; // 完整的错误信息
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
    std::string s;                        // token内容
    int line;                             // 行号
    int pos;                              // 第几个字符
    bool isBaseOperator;                  // 是否为基本运算符（单个字符的运算符以及左右括号）
    Node node;                            // 节点
    std::shared_ptr<std::string> content; // 整行文本
    Token(int line, int pos, bool isBaseOperator, const std::string &s, const std::shared_ptr<std::string> &content)
        : s(s), line(line), pos(pos), isBaseOperator(isBaseOperator), content(content) {}
};

class ParseFunctions {
public:
    /**
     * 解析表达式字符串为in order记号流。其实就是做词法分析。
     * @exception ParseError
     */
    static std::deque<Token> ParseToTokens(const std::string &expression);

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
Node Parse(const std::string &expression);

} // namespace tomsolver

using std::string;
using std::to_string;

namespace tomsolver {

inline SingleParseError::SingleParseError(int line, int pos, const std::string &content, const std::string &errInfo)
    : line(line), pos(pos), content(content), errInfo(errInfo) {
    whatStr = "[Parse Error] " + errInfo + " at(" + to_string(line) + ", " + to_string(pos) + "):\n";
    whatStr += content + "\n";
    whatStr += string(pos, ' ') + "^---- error position";
}

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
    for (auto &err : parseErrors) {
        whatStr += std::string(err.what()) + "\n";
    }
}

const char *MultiParseError::what() const noexcept {
    return whatStr.c_str();
}

/* 是基本运算符()+-* /^&|% */
bool IsBaseOperator(char c) noexcept {
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
MathOperator BaseOperatorCharToEnum(char c) noexcept {
    switch (c) {
    case '(':
        return MathOperator::MATH_LEFT_PARENTHESIS;
    case ')':
        return MathOperator::MATH_RIGHT_PARENTHESIS;
    case '+':
        return MathOperator::MATH_ADD;
    case '-':
        return MathOperator::MATH_SUB;
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

MathOperator Str2Function(const std::string &s) noexcept {
    if (s == "sin") {
        return MathOperator::MATH_SIN;
    }
    if (s == "cos") {
        return MathOperator::MATH_COS;
    }
    if (s == "tan") {
        return MathOperator::MATH_TAN;
    }
    if (s == "arcsin") {
        return MathOperator::MATH_ARCSIN;
    }
    if (s == "arccos") {
        return MathOperator::MATH_ARCCOS;
    }
    if (s == "arctan") {
        return MathOperator::MATH_ARCTAN;
    }
    if (s == "sqrt") {
        return MathOperator::MATH_SQRT;
    }
    if (s == "log") {
        return MathOperator::MATH_LOG;
    }
    if (s == "log2") {
        return MathOperator::MATH_LOG2;
    }
    if (s == "log10") {
        return MathOperator::MATH_LOG10;
    }
    if (s == "exp") {
        return MathOperator::MATH_EXP;
    }
    return MathOperator::MATH_NULL;
}

namespace internal {

// 粗切分：利用operator切分
std::vector<Token> SplitRough(const std::string &expression) {
    std::shared_ptr<std::string> content = std::make_shared<std::string>(expression);
    std::vector<Token> ret;

    int tempBeg = 0;
    std::string temp;
    for (int i = 0; i < static_cast<int>(expression.size()); ++i) {
        char c = expression[i];

        // 忽略tab (\t) whitespaces (\n, \v, \f, \r) space
        if (isspace(c)) {
            continue;
        }

        if (!IsBaseOperator(c)) {
            temp.push_back(c);
        } else {
            if (!temp.empty()) {
                ret.push_back(Token(0, tempBeg, false, temp, content));
                tempBeg = i;

                temp.clear();
            }
            ret.push_back(Token(0, tempBeg, true, std::string{c}, content));
            tempBeg = i + 1;
        }
    }
    if (!temp.empty()) {
        ret.push_back(Token(0, tempBeg, false, temp, content));
        temp.clear();
    }

    return ret;
}

std::deque<Token> ParseFunctions::ParseToTokens(const std::string &expression) {

    if (expression.empty()) {
        throw SingleParseError(0, 0, "empty input", expression);
    }

    std::vector<Token> tokens = SplitRough(expression);

    std::deque<Token> ret;
    // 二次切分：切分出3类元素
    for (size_t i = 0; i < tokens.size(); i++) {
        Token token = std::move(tokens[i]);
        auto &s = token.s;
        if (token.isBaseOperator) // 识别出基本运算符（括号也在其中）
        {
            token.node = std::make_unique<NodeImpl>(NodeType::OPERATOR, BaseOperatorCharToEnum(s[0]), 0, "");
            ret.push_back(std::move(token));
            continue;
        }

        // 检验是否为浮点数
        try {
            std::size_t sz;
            double d = std::stod(s, &sz);
            if (sz != s.size()) {
                throw std::invalid_argument("");
            }
            token.node = Num(d);
            ret.push_back(std::move(token));
            continue;
        } catch (const std::exception &) {}

        // 识别出函数
        MathOperator op = Str2Function(s);
        if (op != MathOperator::MATH_NULL) {
            token.node = std::make_unique<NodeImpl>(NodeType::OPERATOR, op, 0, "");
            ret.push_back(std::move(token));
            continue;
        }

        // 变量
        // 非运算符、数字、函数
        if (!VarNameIsLegal(s)) // 变量名首字符需为下划线或字母
        {
            throw SingleParseError(token.line, token.pos, expression, "Invalid variable name: \"" + s + "\"");
        }

        token.node = Var(s);
        ret.push_back(std::move(token));
    }
    // 此时3类元素均已切分

    // 识别取正运算符与取负运算符
    size_t i = 0;
    if (ret[0].node->op == MathOperator::MATH_ADD) {
        ret[0].node->op = MathOperator::MATH_POSITIVE;
        i++;
    }
    if (ret[0].node->op == MathOperator::MATH_SUB) {
        ret[0].node->op = MathOperator::MATH_NEGATIVE;
        i++;
    }
    for (; i < ret.size();) {
        if (ret[i].node->type == NodeType::OPERATOR && ret[i].node->op != MathOperator::MATH_RIGHT_PARENTHESIS) {
            if (i + 1 < ret.size())
                i++;
            else
                break;
            if (ret[i].node->op == MathOperator::MATH_ADD) {
                ret[i].node->op = MathOperator::MATH_POSITIVE;
                i++;
                continue;
            }
            if (ret[i].node->op == MathOperator::MATH_SUB) {
                ret[i].node->op = MathOperator::MATH_NEGATIVE;
                i++;
                continue;
            }
        } else
            i++;
    }

    return ret;
}

std::vector<Token> ParseFunctions::InOrderToPostOrder(std::deque<Token> &inOrder) {
    std::vector<Token> postOrder;
    int parenthesisBalance = 0;
    std::stack<Token> temp;
    while (inOrder.size() > 0) {
        Token &f = inOrder.front();

        // 数字直接入栈
        if (f.node->type == NodeType::NUMBER || f.node->type == NodeType::VARIABLE) {
            postOrder.push_back(std::move(f));
            inOrder.pop_front();
            continue;
        }

        //(左括号直接入栈
        if (f.node->op == MathOperator::MATH_LEFT_PARENTHESIS) {
            temp.push(std::move(f));
            inOrder.pop_front();
            parenthesisBalance++;
            continue;
        }

        if (f.node->op == MathOperator::MATH_RIGHT_PARENTHESIS) //)出现右括号
        {
            parenthesisBalance--;

            // 括号balance<0，说明括号不匹配
            if (parenthesisBalance < 0) {
                throw SingleParseError(f.line, f.pos, *f.content, "Parenthesis not match: \"" + f.s + "\"");
            }

            // pop至左括号
            while (temp.size() > 0) {
                if (temp.top().node->op == MathOperator::MATH_LEFT_PARENTHESIS) //(
                {
                    temp.pop(); // 扔掉左括号
                    break;
                } else {
                    postOrder.push_back(std::move(temp.top())); // 入队
                    temp.pop();
                }
            }

            // 取出函数
            if (temp.size() > 0 && IsFunction(temp.top().node->op)) {
                postOrder.push_back(std::move(temp.top()));
                temp.pop();
            }

            // pop所有取正取负
            while (temp.size() > 0) {
                if (temp.top().node->op == MathOperator::MATH_POSITIVE ||
                    temp.top().node->op == MathOperator::MATH_NEGATIVE) {
                    postOrder.push_back(std::move(temp.top()));
                    temp.pop();
                } else
                    break;
            }
            inOrder.pop_front(); // 扔掉右括号
            continue;
        }

        // f不是括号
        if (f.node->op == MathOperator::MATH_POSITIVE || f.node->op == MathOperator::MATH_NEGATIVE) {
            temp.push(std::move(f));
            inOrder.pop_front();
            continue;
        }

        // 不是括号也不是正负号
        if (temp.size() > 0 && IsLeft2Right(temp.top().node->op) == true) // 左结合
            // 临时栈有内容，且新进符号优先级低，则挤出高优先级及同优先级符号
            while (temp.size() > 0 && Rank(f.node->op) <= Rank(temp.top().node->op)) {
                postOrder.push_back(std::move(temp.top())); // 符号进入post队列
                temp.pop();
            }
        else
            // 右结合
            // 临时栈有内容，且新进符号优先级低，则挤出高优先级，但不挤出同优先级符号（因为右结合）
            while (temp.size() > 0 && Rank(f.node->op) < Rank(temp.top().node->op)) {
                postOrder.push_back(std::move(temp.top())); // 符号进入post队列
                temp.pop();
            };

        temp.push(std::move(f)); // 高优先级已全部挤出，当前符号入栈
        inOrder.pop_front();
    }

    // 剩下的元素全部入栈
    while (temp.size() > 0) {
        Token token = std::move(temp.top());
        temp.pop();

        // 退栈时出现左括号，说明没有找到与之匹配的右括号
        if (token.node->op == MathOperator::MATH_LEFT_PARENTHESIS) {
            throw SingleParseError(token.line, token.pos, *token.content, "Parenthesis not match: \"" + token.s + "\"");
        }

        postOrder.push_back(std::move(token));
    }

    return postOrder;
}

// 将PostOrder建立为树，并进行表达式有效性检验（确保二元及一元运算符、函数均有操作数）
Node ParseFunctions::BuildExpressionTree(std::vector<Token> &postOrder) {
    std::stack<Token> tempStack;
    // 逐个识别PostOrder序列，构建表达式树
    for (auto &token : postOrder) {
        switch (token.node->type) {
        case NodeType::NUMBER:
        case NodeType::VARIABLE:
            tempStack.push(std::move(token));
            break;
        case NodeType::OPERATOR:
            if (GetOperatorNum(token.node->op) == 2) {
                if (tempStack.empty()) {
                    throw MathError{ErrorType::ERROR_WRONG_EXPRESSION, ""};
                }

                tempStack.top().node->parent = token.node.get();
                token.node->right = std::move(tempStack.top().node);
                tempStack.pop();

                if (tempStack.empty()) {
                    throw MathError{ErrorType::ERROR_WRONG_EXPRESSION, ""};
                }

                tempStack.top().node->parent = token.node.get();
                token.node->left = std::move(tempStack.top().node);
                tempStack.pop();

                tempStack.push(std::move(token));
                continue;
            }

            // 一元运算符
            assert(GetOperatorNum(token.node->op) == 1);

            if (tempStack.empty()) {
                throw MathError{ErrorType::ERROR_WRONG_EXPRESSION, ""};
            }

            tempStack.top().node->parent = token.node.get();
            token.node->left = std::move(tempStack.top().node);
            tempStack.pop();

            tempStack.push(std::move(token));

            break;
        }
    }

    // 如果现在临时栈里面有超过1个元素，那么除了栈顶，其他的都代表出错
    if (tempStack.size() > 1) {
        // 扔掉最顶上的，构造到一半的表达式
        tempStack.pop();

        std::vector<SingleParseError> errors;
        while (!tempStack.empty()) {
            Token &token = tempStack.top();
            errors.push_back(
                SingleParseError(token.line, token.pos, *token.content, "Parse Error at: \"" + token.s + "\""));
            tempStack.pop();
        }
        std::reverse(errors.begin(), errors.end());
        throw MultiParseError(errors);
    }

    return std::move(tempStack.top().node);
}

} // namespace internal

Node Parse(const std::string &expression) {
    std::deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens(expression);
    auto postOrder = internal::ParseFunctions::InOrderToPostOrder(tokens);
    auto node = internal::ParseFunctions::BuildExpressionTree(postOrder);
    return node;
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
 * 变量名通过分析equations得到。初值通过GetConfig()得到。
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

    if (GetConfig().logLevel >= LogLevel::TRACE) {
        cout << "Jacobian = " << jaEqs.ToString() << endl;
    }

    while (1) {
        Vec phi = equations.Clone().Subs(table).Calc().ToMat().ToVec();
        if (GetConfig().logLevel >= LogLevel::TRACE) {
            cout << "iteration = " << it << endl;
            cout << "phi = " << phi << endl;
        }

        if (phi == 0) {
            break;
        }

        if (it > GetConfig().maxIterations) {
            throw runtime_error("迭代次数超出限制");
        }

        Mat ja = jaEqs.Clone().Subs(table).Calc().ToMat();

        Vec deltaq = SolveLinear(ja, -phi);

        q += deltaq;

        if (GetConfig().logLevel >= LogLevel::TRACE) {
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

    if (GetConfig().logLevel >= LogLevel::TRACE) {
        cout << "Jacobi = " << JaEqs << endl;
    }

    while (1) {
        if (GetConfig().logLevel >= LogLevel::TRACE) {
            cout << "iteration = " << it << endl;
        }

        double mu = 1e-5; // LM方法的λ值

        Vec F = equations.Clone().Subs(table).Calc().ToMat().ToVec(); // 计算F

        if (GetConfig().logLevel >= LogLevel::TRACE) {
            cout << "F = " << F << endl;
        }

        if (F == 0) // F值为0，满足方程组求根条件
            goto success;

        Vec FNew(n);   // 下一轮F
        Vec deltaq(n); // Δq
        while (1) {

            Mat J = JaEqs.Clone().Subs(table).Calc().ToMat(); // 计算雅可比矩阵

            if (GetConfig().logLevel >= LogLevel::TRACE) {
                cout << "J = " << J << endl;
            }

            // 说明：
            // 标准的LM方法中，d=-(J'*J+λI)^(-1)*J'F，其中J'*J是为了确保矩阵对称正定。但实践发现此时的d过大，很难收敛。
            // 所以只用了牛顿法的 d=-(J+λI)^(-1)*F

            // enumError err = SolveLinear(J.Transpose()*J + mu * Matrix(J.rows, J.cols).Ones(), d, -J.Transpose()*F);

            // 方向向量
            Vec d = SolveLinear(J + mu * Mat(J.Rows(), J.Cols()).Ones(), -F); // 得到d

            if (GetConfig().logLevel >= LogLevel::TRACE) {
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

            if (GetConfig().logLevel >= LogLevel::TRACE) {
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

            if (it++ == GetConfig().maxIterations)
                goto overIterate;
        }

        q += deltaq; // 应用Δq，更新q值

        table.SetValues(q);

        F = FNew; // 更新F

        if (it++ == GetConfig().maxIterations)
            goto overIterate;

        if (GetConfig().logLevel >= LogLevel::TRACE) {
            cout << std::string(20, '=') << endl;
        }
    }

success:
    if (GetConfig().logLevel >= LogLevel::TRACE) {
        cout << "success" << endl;
    }
    return table;

overIterate:
    throw runtime_error("迭代次数超出限制");
}

VarsTable Solve(const VarsTable &varsTable, const SymVec &equations) {
    switch (GetConfig().nonlinearMethod) {
    case NonlinearMethod::NEWTON_RAPHSON:
        return SolveByNewtonRaphson(varsTable, equations);
    case NonlinearMethod::LM:
        return SolveByLM(varsTable, equations);
    }
    throw runtime_error("invalid config.NonlinearMethod value: " +
                        std::to_string(static_cast<int>(GetConfig().nonlinearMethod)));
}

VarsTable Solve(const SymVec &equations) {
    auto varNames = equations.GetAllVarNames();
    std::vector<std::string> vecVarNames(varNames.begin(), varNames.end());
    VarsTable varsTable(std::move(vecVarNames), GetConfig().initialValue);
    return Solve(varsTable, equations);
}

} // namespace tomsolver

namespace tomsolver {

namespace internal {} // namespace internal

SymMat::SymMat(int rows, int cols) noexcept {
    assert(rows > 0 && cols > 0);
    data.resize(rows);
    for (auto &row : data) {
        row.resize(cols);
    }
}

SymMat::SymMat(const std::initializer_list<std::initializer_list<Node>> &lst) noexcept
    : SymMat(static_cast<int>(lst.size()), static_cast<int>(lst.begin()->size())) {
    for (auto it1 = lst.begin(); it1 != lst.end(); ++it1) {
        std::size_t i = it1 - lst.begin();
        for (auto it2 = it1->begin(); it2 != it1->end(); ++it2) {
            std::size_t j = it2 - it1->begin();
            auto &node = const_cast<Node &>(*it2);
            data[i][j] = std::move(node);
        }
    }
}

SymMat::SymMat(const Mat &rhs) noexcept : SymMat(rhs.Rows(), rhs.Cols()) {
    for (int i = 0; i < rhs.Rows(); ++i) {
        for (int j = 0; j < rhs.Cols(); ++j) {
            data[i][j] = Num(rhs[i][j]);
        }
    }
}

SymMat SymMat::Clone() const noexcept {
    SymMat ret(Rows(), Cols());
    for (int i = 0; i < Rows(); ++i) {
        for (int j = 0; j < Cols(); ++j) {
            ret.data[i][j] = tomsolver::Clone(data[i][j]);
        }
    }
    return ret;
}

bool SymMat::Empty() const noexcept {
    return data.empty();
}

int SymMat::Rows() const noexcept {
    return static_cast<int>(data.size());
}

int SymMat::Cols() const noexcept {
    if (Rows()) {
        return static_cast<int>(data[0].size());
    }
    return 0;
}

SymVec SymMat::ToSymVec() const {
    assert(Rows() > 0);
    if (Cols() != 1) {
        throw std::runtime_error("SymMat::ToSymVec fail. rows is not one");
    }
    SymVec v(Rows());
    for (int j = 0; j < Rows(); ++j) {
        v.data[j][0] = tomsolver::Clone(data[j][0]);
    }
    return v;
}

SymVec SymMat::ToSymVecOneByOne() const noexcept {
    SymVec ans(Rows() * Cols());
    int index = 0;
    for (auto &row : data) {
        for (auto &node : row) {
            ans[index++] = tomsolver::Clone(node);
        }
    }
    return ans;
}

Mat SymMat::ToMat() const {
    std::vector<std::vector<double>> arr(Rows(), std::vector<double>(Cols()));
    for (int i = 0; i < Rows(); ++i) {
        for (int j = 0; j < Cols(); ++j) {
            if (data[i][j]->type != NodeType::NUMBER) {
                throw std::runtime_error("ToMat error: node is not number");
            }
            arr[i][j] = data[i][j]->value;
        }
    }
    return Mat(std::move(arr));
}

SymMat &SymMat::Calc() {
    for (auto &row : data) {
        for (auto &node : row) {
            node->Calc();
        }
    }
    return *this;
}

SymMat &SymMat::Subs(const std::map<std::string, double> &varValues) noexcept {
    for (auto &row : data) {
        for (auto &node : row) {
            node = tomsolver::Subs(std::move(node), varValues);
        }
    }
    return *this;
}

SymMat &SymMat::Subs(const VarsTable &varsTable) noexcept {
    for (auto &row : data) {
        for (auto &node : row) {
            node = tomsolver::Subs(std::move(node), varsTable);
        }
    }
    return *this;
}

std::set<std::string> SymMat::GetAllVarNames() const noexcept {
    std::set<std::string> ret;
    for (int i = 0; i < Rows(); ++i) {
        for (int j = 0; j < Cols(); ++j) {
            auto varNames = data[i][j]->GetAllVarNames();
            ret.insert(varNames.begin(), varNames.end());
        }
    }
    return ret;
}

SymMat SymMat::operator-(const SymMat &rhs) const noexcept {
    assert(rhs.Rows() == Rows() && rhs.Cols() == Cols());
    SymMat ret(Rows(), Cols());
    for (int i = 0; i < Rows(); ++i) {
        for (int j = 0; j < Cols(); ++j) {
            ret.data[i][j] = tomsolver::Clone(data[i][j]) - tomsolver::Clone(rhs.data[i][j]);
        }
    }
    return ret;
}

SymMat SymMat::operator*(const SymMat &rhs) const {
    if (Cols() != rhs.Rows()) {
        throw MathError(ErrorType::SIZE_NOT_MATCH, "");
    }
    SymMat ans(Rows(), rhs.Cols());
    for (int i = 0; i < Rows(); ++i) {
        for (int j = 0; j < rhs.Cols(); ++j) {
            Node sum = data[i][0] * rhs[0][j];
            for (int k = 1; k < Cols(); ++k) {
                sum += data[i][k] * rhs[k][j];
            }
            ans[i][j] = Move(sum);
        }
    }
    return ans;
}

bool SymMat::operator==(const SymMat &rhs) const noexcept {
    if (rhs.Rows() != Rows() || rhs.Cols() != Cols()) {
        return false;
    }
    for (int i = 0; i < Rows(); ++i) {
        for (int j = 0; j < Cols(); ++j) {
            if (!data[i][j]->Equal(rhs.data[i][j])) {
                return false;
            }
        }
    }
    return true;
}

const std::vector<Node> &SymMat::operator[](int row) const noexcept {
    assert(row < Rows());
    return data[row];
}

std::vector<Node> &SymMat::operator[](int row) noexcept {
    assert(row < Rows());
    return data[row];
}

std::string SymMat::ToString() const noexcept {
    if (Empty())
        return "[]";
    std::string s;
    s.reserve(256);

    auto OutputRow = [&](int i) {
        int j = 0;
        for (; j < Cols() - 1; ++j) {
            s += data[i][j]->ToString() + ", ";
        }
        s += data[i][j]->ToString();
    };

    s += "[";
    OutputRow(0);
    s += "\n";

    int i = 1;
    for (; i < Rows() - 1; ++i) {
        s += " ";
        OutputRow(i);
        s += "\n";
    }
    s += " ";
    OutputRow(i);
    s += "]";
    return s;
}

SymVec::SymVec(int rows) noexcept : SymMat(rows, 1) {}

SymVec::SymVec(const std::initializer_list<Node> &lst) noexcept : SymMat(static_cast<int>(lst.size()), 1) {
    for (auto it = lst.begin(); it != lst.end(); ++it) {
        auto &node = const_cast<Node &>(*it);
        data[it - lst.begin()][0] = std::move(node);
    }
}

SymVec SymVec::operator-(const SymVec &rhs) const noexcept {
    return SymMat::operator-(rhs).ToSymVec();
}

Node &SymVec::operator[](std::size_t index) noexcept {
    return data[index][0];
}

const Node &SymVec::operator[](std::size_t index) const noexcept {
    return data[index][0];
}

SymMat Jacobian(const SymMat &equations, const std::vector<std::string> &vars) noexcept {
    int rows = equations.Rows();
    int cols = static_cast<int>(vars.size());
    SymMat ja(rows, cols);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            ja.data[i][j] = Diff(equations.data[i][0], vars[j]);
        }
    }
    return ja;
}

std::ostream &operator<<(std::ostream &out, const SymMat &symMat) noexcept {
    return out << symMat.ToString();
}

} // namespace tomsolver

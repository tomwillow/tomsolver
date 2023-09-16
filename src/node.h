#include "error_type.h"

#include <iostream>

#include <vector>

#include <memory>
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
    MATH_SUBSTRACT,
    MATH_MULTIPLY,
    MATH_DIVIDE,
    MATH_POWER,
    MATH_AND,
    MATH_OR,
    MATH_MOD,

    MATH_LEFT_PARENTHESIS,
    MATH_RIGHT_PARENTHESIS
};

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
    case MathOperator::MATH_SUBSTRACT:
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
    throw MathError{ErrorType::ERROR_WRONG_MATH_OPERATOR,
                             std::string("value=" + std::to_string(static_cast<int>(op)))};
}

enum class NodeType { NUMBER, OPERATOR, VARIABLE, FUNCTION };

/* 单个元素 */
struct Node {

    Node(NodeType type, MathOperator op, double value, std::string varname)
        : type(type), op(op), value(value), varname(varname), parent(nullptr) {}

    Node(const Node &rhs) {
        operator=(rhs);
    }

    Node &operator=(const Node &rhs) {
        type = rhs.type;
        op = rhs.op;
        value = rhs.value;
        varname = rhs.varname;
        parent = rhs.parent;
        left = CopyTree(rhs.left);
        right = CopyTree(rhs.right);
        return *this;
    }

private:
    NodeType type;
    MathOperator op;
    double value;
    std::string varname;
    Node *parent;
    std::unique_ptr<Node> left, right;

    std::unique_ptr<Node> CopyTree(const std::unique_ptr<Node> &rhs) const noexcept {
        auto ret = std::make_unique<Node>(rhs->type, rhs->op, rhs->value, rhs->varname);
        ret->left = CopyTree(rhs->left);
        ret->right = CopyTree(rhs->right);
        return ret;
    }

    std::string ToString() const noexcept {
        switch (type) {
        case NodeType::NUMBER:
            return std::to_string(value);
        case NodeType::VARIABLE:
            return varname;
        default:
            assert(0 && "unexpected NodeType. maybe this is a bug.");
        }
    }

     void TraverseInOrder(Node *now, std::string &output)
    {
    	int has_parenthesis = 0;
    	if (GetOperateNum(now->eOperator) == 1)//一元运算符：函数和取负
    	{
    		if (now->eType == NODE_FUNCTION)
    		{
    			output += Node2Str(*now) + "(";
    			has_parenthesis = 1;
    		}
    		else
    		{
    			output += "(" + Node2Str(*now);
    			has_parenthesis = 1;
    		}
    	}
    
    	if (GetOperateNum(now->eOperator) != 1)//非一元运算符才输出，即一元运算符的输出顺序已改变
    	{
    		if (now->eType == NODE_OPERATOR)//本级为运算符
    			if (now->parent != NULL)
    				if (
    					(GetOperateNum(now->parent->eOperator) == 2 &&//父运算符存在，为二元，
    						(
    							Rank(now->parent->eOperator) >
    Rank(now->eOperator)//父级优先级高于本级->加括号
    							||
    							(//两级优先级相等
    								Rank(now->parent->eOperator) == Rank(now->eOperator) &&
    								(
    									//本级为父级的右子树 且父级不满足结合律->加括号
    									(inAssociativeLaws(now->parent->eOperator) == false && now ==
    now->parent->right)
    									||
    									////两级都是右结合
    									(isLeft2Right(now->parent->eOperator) == false && isLeft2Right(now->eOperator) ==
    false)
    									)
    								)
    							)
    						)
    
    					//||
    
    					////父运算符存在，为除号，且本级为分子，则添加括号
    					//(now->parent->eOperator == MATH_DIVIDE && now == now->parent->right)
    					)
    				{
    					output += "(";
    					has_parenthesis = 1;
    				}
    	}
    
    	if (now->left != NULL)//左遍历
    	{
    		TraverseInOrder(now->left, output);
    	}
    
    	if (GetOperateNum(now->eOperator) != 1)//非一元运算符才输出，即一元运算符的输出顺序已改变
    	{
    		output += Node2Str(*now);
    	}
    
    
    	if (now->right != NULL)//右遍历
    	{
    		TraverseInOrder(now->right, output);
    	}
    
    	//回到本级时补齐右括号，包住前面的东西
    	if (has_parenthesis)
    	{
    		output += "";
    	}
    }

    friend std::ostream &operator<<(std::ostream &out, const std::unique_ptr<Node> &n) noexcept;
    friend std::unique_ptr<Node> operator+(const std::unique_ptr<Node> &n1, const std::unique_ptr<Node> &n2) noexcept;
    friend std::unique_ptr<Node> operator+(std::unique_ptr<Node> &&n1, std::unique_ptr<Node> &&n2) noexcept;
};

std::ostream &operator<<(std::ostream &out, const std::unique_ptr<Node> &n) noexcept {
    if (n->left) {
        out << n->left;
    }

    out << n->ToString() << " ";

    if (n->right) {
        out << n->right;
    }

    return out;
}

std::unique_ptr<Node> Num(double num) noexcept {
    return std::make_unique<Node>(NodeType::NUMBER, MathOperator::MATH_NULL, num, "");
}

std::unique_ptr<Node> operator+(const std::unique_ptr<Node> &n1, const std::unique_ptr<Node> &n2) noexcept {
    auto ret = std::make_unique<Node>(NodeType::OPERATOR, MathOperator::MATH_ADD, 0, "");
    auto n1Clone = std::unique_ptr<Node>(new Node(*n1));
    n1Clone->parent = ret.get();
    ret->left = std::move(n1Clone);

    auto n2Clone = std::make_unique<Node>(*n2);
    n2Clone->parent = ret.get();
    ret->left = std::move(n2Clone);
    return ret;
}

std::unique_ptr<Node> operator+(std::unique_ptr<Node> &&n1, std::unique_ptr<Node> &&n2) noexcept {
    auto ret = std::make_unique<Node>(NodeType::OPERATOR, MathOperator::MATH_ADD, 0, "");
    n1->parent = ret.get();
    ret->left = std::move(n1);

    n2->parent = ret.get();
    ret->right = std::move(n2);
    return ret;
}

// Node operator*(const Node &n1, const Node &n2) {
//    Func fn(Func::FuncType::MUL);
//    fn.SetLeft(n1);
//    fn.SetRight(n2);
//    return fn;
//}
//
// class Var : public Node {
// public:
//    explicit Var(const std::string &varname) : varname(varname) {}
//
// private:
//    std::string varname;
//};
//
// class Func : public Node {
// public:
//    enum class FuncType { MUL, SIN, COS };
//    Func(FuncType type) : type(type) {}
//    Func(const std::string &funcName) {
//        if (funcName == "sin") {
//            type = FuncType::SIN;
//        }
//    }
//
// private:
//    FuncType type;
//};
//
// class Matrix {
// public:
// private:
//};

} // namespace tomsolver

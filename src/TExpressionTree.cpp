//#include "TExpressionTree.h"
//
//#include "error_type.h"
//
//#include <string>
//#define _USE_MATH_DEFINES
//#include <math.h>
//
//#include <cassert>
//
// using namespace std;
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
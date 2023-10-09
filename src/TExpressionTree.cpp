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
// namespace tomsolver {
//
// TExpressionTree::TExpressionTree()
//{
//	Reset();
//}
//
//
// TExpressionTree::~TExpressionTree()
//{
//	Release();
//}
//
// void TExpressionTree::Reset()
//{
//	head = NULL;
//	pVariableTable = NULL;
//
//	iVarAppearedCount = 0;
//}
//
// void TExpressionTree::Release()
//{
//	DeleteNode(head);
//	head = NULL;
//}
//
////删除node指向对象 可删除任意位置节点，如被删节点存在父节点则父节点左右儿子置0
// void TExpressionTree::DeleteNode(TNode *node)
//{
//	if (node != NULL)
//	{
//		if (node->parent != NULL)
//		{
//			if (node->parent->left == node)
//				node->parent->left = NULL;
//			if (node->parent->right == node)
//				node->parent->right = NULL;
//		}
//
//		DeleteNodeTraversal(node);
//	}
//}
//
////遍历删除节点 未处理父节点，未判断左右儿子是否存在
// void TExpressionTree::DeleteNodeTraversal(TNode *node)
//{
//	if (node->left != NULL)
//		DeleteNode(node->left);
//	if (node->right != NULL)
//		DeleteNode(node->right);
//
//	delete node;
//}
//
////将PostOrder建立为树，并进行表达式有效性检验（确保二元及一元运算符、函数均有操作数）
// void TExpressionTree::BuildExpressionTree(std::vector<TNode *> &PostOrder)
//{
//	std::stack<TNode *> tempStack;
//	//逐个识别PostOrder序列，构建表达式树
//	for (auto pNodeNow : PostOrder)
//	{
//		switch (pNodeNow->eType)
//		{
//		case NODE_NUMBER:
//		case NODE_VARIABLE:
//			tempStack.push(pNodeNow);
//			break;
//		case NODE_FUNCTION:
//		case NODE_OPERATOR:
//			if (GetOperateNum(pNodeNow->eOperator) == 2)
//			{
//				if (tempStack.empty())
//				{
//					//释放所有TNode，报错
//					ReleaseVectorTNode(PostOrder);
//					//Stack只是对PostOrder的重排序，所以不用delete
//                                        throw MathError{ErrorType::ERROR_WRONG_EXPRESSION, ""};
//					return;
//				}
//
//				pNodeNow->right = tempStack.top();
//				tempStack.top()->parent = pNodeNow;
//				tempStack.pop();
//
//				if (tempStack.empty())
//				{
//					//释放所有TNode，报错
//					ReleaseVectorTNode(PostOrder);
//					//Stack只是对PostOrder的重排序，所以不用delete
//					throw MathError{ ErrorType::ERROR_WRONG_EXPRESSION, "" };
//					return;
//				}
//
//				pNodeNow->left = tempStack.top();
//				tempStack.top()->parent = pNodeNow;
//				tempStack.pop();
//
//				tempStack.push(pNodeNow);
//			}
//			else
//			{
//				if (tempStack.empty())
//				{
//					//释放所有TNode，报错
//					ReleaseVectorTNode(PostOrder);
//					//Stack只是对PostOrder的重排序，所以不用delete
//					throw MathError{ ErrorType::ERROR_WRONG_EXPRESSION, "" };
//					return;
//				}
//
//				pNodeNow->left = tempStack.top();
//				tempStack.top()->parent = pNodeNow;
//				tempStack.pop();
//
//				tempStack.push(pNodeNow);
//			}
//			break;
//		}
//	}
//
//	//找出root
//	head = PostOrder[0];
//	while (head->parent != NULL)
//	{
//		head = head->parent;
//	}
//}
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
// std::string TExpressionTree::Node2Str(const TNode &node)
//{
//	switch (node.eType)
//	{
//	case NODE_NUMBER:
//		if (std::abs(node.value - (long long)node.value) < MIN_DOUBLE)
//			return ToString((long long)node.value);
//		else
//			return ToString(node.value);
//		break;
//	case NODE_VARIABLE:
//		return node.varname;
//		break;
//	case NODE_FUNCTION:
//		return Function2Str(node.eOperator);
//		break;
//	case NODE_OPERATOR:
//		return EnumOperatorToTChar(node.eOperator);
//		break;
//	}
//	assert(0);
//	return "Error";
//}
//
//
// void TExpressionTree::GetNodeNum(TNode *now, int &n)
//{
//	if (now->left != NULL)
//		GetNodeNum(now->left, n);
//	if (now->right != NULL)
//		GetNodeNum(now->right, n);
//	n++;
//}
//
// int TExpressionTree::GetNodeNum(TNode *head)
//{
//	int num = 0;
//	if (head != 0)
//	{
//		GetNodeNum(head, num);
//		return num;
//	}
//	else
//		return 0;
//}
//
// std::string  TExpressionTree::OutputStr()
//{
//	std::string temp;
//
//	if (head != NULL)
//		TraverseInOrder(head, temp);
//	return temp;
//}
//
//
//
//
////复制节点树，返回新节点树头节点
// TExpressionTree::TNode *TExpressionTree::CopyNodeTree(TNode *oldNode)
//{
//	TNode *newNode = new TNode;
//	newNode->eType = oldNode->eType;
//	newNode->eOperator = oldNode->eOperator;
//	newNode->value = oldNode->value;
//	newNode->varname = oldNode->varname;
//
//	if (oldNode->left != NULL)
//	{
//		newNode->left = CopyNodeTree(oldNode->left);
//		newNode->left->parent = newNode;
//	}
//	if (oldNode->right != NULL)
//	{
//		newNode->right = CopyNodeTree(oldNode->right);
//		newNode->right->parent = newNode;
//	}
//
//	return newNode;
//}
//
// TExpressionTree::TNode *TExpressionTree::NewNode(enumNodeType eType, enumMathOperator eOperator)
//{
//	TNode *newNode = new TNode;
//	newNode->eType = eType;
//	newNode->eOperator = eOperator;
//	return newNode;
//}
//
//
// std::string  TExpressionTree::Diff(std::string var, int n, bool bOutput)
//{
//	if (pVariableTable->FindVariableTable(var) == pVariableTable->VariableTable.end())
//		throw MathError{ ErrorType::ERROR_UNDEFINED_VARIABLE, var };
//
//	for (int i = 0; i < n; i++)
//	{
//		Diff(head, var);
//	}
//	return OutputStr();
//}
//
// void TExpressionTree::LinkParent(TNode *child, TNode *ignore)
//{
//	if (ignore == head)
//	{
//		head = child;
//		child->parent = NULL;
//	}
//	else
//	{
//		if (ignore->parent->left == ignore)
//			ignore->parent->left = child;
//		if (ignore->parent->right == ignore)
//			ignore->parent->right = child;
//		child->parent = ignore->parent;
//	}
//}
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
// void  TExpressionTree::Simplify(bool bOutput)
//{
//	Simplify(head);
//}
//
////string  TExpressionTree::FindVariableTableFrom(const string varstr, std::vector<string > newVariableTable)
////{
////	for (auto szNewVar : newVariableTable)
////		if (_tcscmp(varstr, szNewVar) == 0)
////			return szNewVar;
////	return NULL;
////}
////
////void TExpressionTree::ReplaceNodeVariable(TNode *now, std::vector<string > &newVariableTable)
////{
////	if (now->left != NULL)
////		ReplaceNodeVariable(now->left, newVariableTable);
////	if (now->right != NULL)
////		ReplaceNodeVariable(now->right, newVariableTable);
////	TCHAR *temp;
////	if (now->eType == NODE_VARIABLE && (temp = FindVariableTableFrom(now->varname.c_str(), newVariableTable)))
////		now->varname = temp;
////}
//
// void TExpressionTree::GetVariablePos(const std::string var, std::vector<TNode *> &VarsPos)
//{
//	GetVariablePos(head, var, VarsPos);
//}
//
// void TExpressionTree::GetVariablePos(TNode *now, const std::string var, std::vector<TNode *> &VarsPos)
//{
//	if (now->eType == NODE_VARIABLE && now->varname == var)
//		VarsPos.push_back(now);
//	if (now->left != NULL)
//		GetVariablePos(now->left, var, VarsPos);
//	if (now->right != NULL)
//		GetVariablePos(now->right, var, VarsPos);
//}
//
// void TExpressionTree::CopyVariableTable(std::vector<std::string > &Dest, const std::vector<std::string > source)
//{
//	Dest.clear();
//	for (auto sz : source)
//		Dest.push_back(sz);
//}
//
////替换 VarsVector变量 NumsVector数字
// void  TExpressionTree::Subs(std::vector<std::string > VarsVector, std::vector<double> NumsVector, bool output)
//{
//	if (VarsVector.size() == NumsVector.size())//替换与被替换元素数目相等
//	{
//		for (size_t i = 0; i < VarsVector.size(); i++)//遍历被替换变量
//		{
//			//查表识别被替换变量
//			auto it = pVariableTable->FindVariableTable(VarsVector[i]);
//			if (it != pVariableTable->VariableTable.end())//已识别出
//			{
//				std::string var = *it;
//				//构建替换节点树
//				TExpressionTree Expr;
//				Expr.LinkVariableTable(pVariableTable);
//				Expr.Read(NumsVector[i], false);
//
//				//得到所有被替换变量的位置
//				std::vector<TNode *> VarsPos;
//				GetVariablePos(var, VarsPos);
//				for (size_t j = 0; j < VarsPos.size(); j++)
//				{
//					TNode *newNode = CopyNodeTree(Expr.head);
//
//					//连接到新节点
//					if (VarsPos[j] != head)
//					{
//						if (VarsPos[j]->parent->left != NULL && VarsPos[j]->parent->left ==
// VarsPos[j]) 							VarsPos[j]->parent->left = newNode;
// if (VarsPos[j]->parent->right != NULL && VarsPos[j]->parent->right == VarsPos[j]) VarsPos[j]->parent->right =
// newNode; 						newNode->parent = VarsPos[j]->parent;
//					}
//					else
//						head = newNode;
//
//					//删掉旧节点
//					delete VarsPos[j];
//				}
//			}
//		}
//	}
//	else
//		throw MathError{ ErrorType::ERROR_SUBS_NOT_EQUAL, "" };
//}
//
// void TExpressionTree::Subs_inner(TNode *node, std::string ptVar, double value)
//{
//	if (node->eType == NODE_VARIABLE && node->varname == ptVar)
//	{
//		node->eType = NODE_NUMBER;
//		node->value = value;
//		return;
//	}
//
//	if (node->left != NULL)
//		Subs_inner(node->left, ptVar, value);
//
//	if (node->right != NULL)
//		Subs_inner(node->right, ptVar, value);
//}
//
////替换  var变量指针 value数字
// void  TExpressionTree::Subs(std::string ptVar, double value, bool output)
//{
//	Subs_inner(head, ptVar, value);
//}
//
////替换  vars变量串 nums数字串 以空格分隔，支持表达式替换
// void  TExpressionTree::Subs(const std::string vars, const std::string nums, bool output)
//{
//	if (vars.empty() || nums.empty())
//	{
//		throw MathError{ ErrorType::ERROR_EMPTY_INPUT, "" };
//		return;
//	}
//
//	std::vector<std::string> VarsVector = StrSliceToVector(vars);
//	std::vector<std::string> NumsVector = StrSliceToVector(nums);
//
//	if (VarsVector.size() == NumsVector.size())//替换与被替换元素数目相等
//	{
//		for (size_t i = 0; i < VarsVector.size(); i++)//遍历被替换变量
//		{
//			//查表识别被替换变量
//			auto it = pVariableTable->FindVariableTable(VarsVector[i]);
//			if (it != pVariableTable->VariableTable.end())//已识别出
//			{
//				std::string var = *it;
//				//构建替换节点树
//				TExpressionTree Expr;
//				Expr.LinkVariableTable(pVariableTable);
//				Expr.Read(NumsVector[i], false);
//
//				//得到所有被替换变量的位置
//				std::vector<TNode *> VarsPos;
//				GetVariablePos(var, VarsPos);
//				for (size_t j = 0; j < VarsPos.size(); j++)
//				{
//					TNode *newNode = CopyNodeTree(Expr.head);
//
//					//连接到新节点
//					if (VarsPos[j] != head)
//					{
//						if (VarsPos[j]->parent->left != NULL && VarsPos[j]->parent->left ==
// VarsPos[j]) 							VarsPos[j]->parent->left = newNode;
// if (VarsPos[j]->parent->right != NULL && VarsPos[j]->parent->right == VarsPos[j]) VarsPos[j]->parent->right =
// newNode; 						newNode->parent = VarsPos[j]->parent;
//					}
//					else
//						head = newNode;
//
//					//删掉旧节点
//					delete VarsPos[j];
//				}
//			}
//		}
//	}
//	else
//		throw MathError{ ErrorType::ERROR_SUBS_NOT_EQUAL, "" };
//
//}
//
// void  TExpressionTree::LinkVariableTable(TVariableTable *p)
//{
//	pVariableTable = p;
//}
//
// void  TExpressionTree::Read(double num, bool bOutput)
//{
//	head = NewNode(NODE_NUMBER);
//	head->value = num;
//}
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
////读之前不清零，请自行处理
// void  TExpressionTree::Read(std::string expression, bool bOutput)
//{
//	std::queue<TNode *> InOrder;
//	std::vector<TNode *> PostOrder;
//
//	ReadToInOrder(expression, InOrder);
//	InQueue2PostQueue(InOrder, PostOrder);
//	BuildExpressionTree(PostOrder);
//}
//
// bool TExpressionTree::CanCalc(TNode *now)
//{
//	if (now->left != NULL)
//		if (CanCalc(now->left) == false)
//			return false;
//	if (now->right != NULL)
//		if (CanCalc(now->right) == false)
//			return false;
//
//	if (now->eType == NODE_VARIABLE)
//		return false;
//	return true;
//}
//
// bool TExpressionTree::CanCalc()
//{
//	return CanCalc(head);
//}
//
////计算表达式值 operateHeadNode决定是否操作本身的节点
// double TExpressionTree::Value(bool operateHeadNode)
//{
//	TNode *pNode = NULL;
//	if (operateHeadNode)
//		pNode = head;
//	else
//		pNode = CopyNodeTree(head);
//
//	try
//	{
//		Calc(pNode);
//	} catch (const MathError &err)
//	{
//		//删掉节点树并提交给上级
//		if (operateHeadNode == false)
//			DeleteNode(pNode);
//		throw err;
//	}
//
//	//得到最终结果
//	double num = pNode->value;
//	//释放复制的树
//	if (operateHeadNode == false)
//		delete pNode;
//	return num;
//}
//
////复制出一棵临时树计算值
// std::string  TExpressionTree::Calc(double *result)
//{
//	if (CanCalc())
//	{
//		TNode *Duplicate = CopyNodeTree(head);
//		Calc(Duplicate);
//
//		if (result != NULL)
//			*result = Duplicate->value;
//
//		std::string temp = Node2Str(*Duplicate);
//		delete Duplicate;
//
//		return temp;
//	}
//	else
//		return OutputStr();
//}
//
// TExpressionTree &TExpressionTree::operator=(const TExpressionTree &expr)
//{
//	Release();
//	Reset();
//	head = CopyNodeTree(expr.head);
//	LinkVariableTable(expr.pVariableTable);
//	return *this;
//}
//
// void TExpressionTree::ReleaseVectorTNode(std::vector<TNode *> vec)
//{
//	for (auto pNode : vec)
//		delete pNode;
//}
//
// void TExpressionTree::Vpa_inner(TNode *now)
//{
//	if (now->left != NULL)
//		Vpa_inner(now->left);
//	if (now->right != NULL)
//		Vpa_inner(now->right);
//
//	if (now->eType == NODE_VARIABLE)
//	{
//		now->eType = NODE_NUMBER;
//		now->value = pVariableTable->GetValueFromVarPoint(now->varname);
//	}
//}
//
////仅将变量表内置数值代入，不进行计算
// void  TExpressionTree::Vpa(bool bOutput)
//{
//	Vpa_inner(head);
//}
//
// bool TExpressionTree::IsSingleVar()//检查是否为一元
//{
//	//return SelfVariableTable.VariableTable.size() == 1;
//	return true;
//}
//
//
// void TExpressionTree::CheckOnlyOneVar(TNode *now)//只有一个变量（实时验证）
//{
//	if (now->eType == NODE_VARIABLE)
//	{
//		iVarAppearedCount++;
//		LastVarNode = now;
//	}
//	if (now->left != NULL)
//		CheckOnlyOneVar(now->left);
//
//	if (now->right != NULL)
//		CheckOnlyOneVar(now->right);
//}
//
// bool TExpressionTree::CheckOnlyOneVar()//只有一个变量（实时验证）
//{
//	iVarAppearedCount = 0;
//	CheckOnlyOneVar(head);
//	return HasOnlyOneVar();
//}
//
// bool TExpressionTree::HasOnlyOneVar()//只有一个变量
//{
//	return iVarAppearedCount == 1;
//}
//
// TExpressionTree &TExpressionTree::operator*(double value)
//{
//	if (head == NULL)
//	{
//		throw MathError{ ErrorType::ERROR_EMPTY_INPUT, "" };
//		return *this;
//	}
//
//	TNode *Multiply = NewNode(NODE_OPERATOR, MATH_MULTIPLY);
//	TNode *Value = NewNode(NODE_NUMBER);
//	Value->value = value;
//
//	Multiply->left = head;
//	Multiply->right = Value;
//
//	head->parent = Multiply;
//	Value->parent = Multiply;
//
//	head = Multiply;
//
//	return *this;
//}
//
// TExpressionTree &TExpressionTree::operator+(double value)
//{
//	if (head == NULL)
//	{
//		throw MathError{ ErrorType::ERROR_EMPTY_INPUT, "" };
//		return *this;
//	}
//
//	TNode *Add = NewNode(NODE_OPERATOR, MATH_ADD);
//	TNode *Value = NewNode(NODE_NUMBER);
//	Value->value = value;
//
//	Add->left = head;
//	Add->right = Value;
//
//	head->parent = Add;
//	Value->parent = Add;
//
//	head = Add;
//
//	return *this;
//}
//
// TExpressionTree &TExpressionTree::operator-(double value)
//{
//	if (head == NULL)
//	{
//		throw MathError{ ErrorType::ERROR_EMPTY_INPUT, "" };
//		return *this;
//	}
//
//	TNode *Substract = NewNode(NODE_OPERATOR, MATH_SUB);
//	TNode *Value = NewNode(NODE_NUMBER);
//	Value->value = value;
//
//	Substract->left = head;
//	Substract->right = Value;
//
//	head->parent = Substract;
//	Value->parent = Substract;
//
//	head = Substract;
//
//	return *this;
//}
//
// TExpressionTree &TExpressionTree::operator+(const TExpressionTree &expr)
//{
//	if (head == NULL)
//		head = CopyNodeTree(expr.head);
//	else
//	{
//		TNode *Add = NewNode(NODE_OPERATOR, MATH_ADD);
//		TNode *Right = CopyNodeTree(expr.head);
//
//
//		Add->left = head;
//		Add->right = Right;
//
//		head->parent = Add;
//		Right->parent = Add;
//
//		head = Add;
//	}
//	return *this;
//}
//
//}
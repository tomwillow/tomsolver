#include "diff.h"

#include "functions.h"

#include <queue>

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
                        assert(0 && "inner bug");
                    }
                } else {
                    if (isLeftChild) {
                        assert(0 && "inner bug");
                    }
                }
            }
        }
    };

    static void DiffOnce(std::unique_ptr<NodeImpl> &root, const std::string &varname) {
        std::queue<DiffNode> q;

        if (root->type == NodeType::OPERATOR) {
            DiffOnceOperator(root, q);
        } else {
            q.push(DiffNode(root.get(), true));
        }

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
            case NodeType::OPERATOR: {
                if (f.isLeftChild) {
                    DiffOnceOperator(const_cast<NodeImpl *>(f.node->parent)->left, q);
                } else {
                    DiffOnceOperator(const_cast<NodeImpl *>(f.node->parent)->right, q);
                }
                break;
            }
            default:
                assert(0 && "inner bug");
            }
        }
    }

    static void DiffOnceOperator(std::unique_ptr<NodeImpl> &node, std::queue<DiffNode> &q) {
        auto parent = node->parent;

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

        switch (node->op) {
        case MathOperator::MATH_NULL: {
            assert(0 && "inner bug");
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
            Node u2 = Clone(node->left);
            q.push(DiffNode(u2.get(), false));
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
            Node u2 = Clone(node->left);
            q.push(DiffNode(u2.get(), false));
            node = -Move(node) * Move(u2);
            node->parent = parent;
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
            Node u2 = Clone(node->left);
            q.push(DiffNode(u2.get(), false));
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
                q.push(DiffNode(node->left.get(), true));
            }
            if (node->right) {
                q.push(DiffNode(node->right.get(), false));
            }
            return;
        case MathOperator::MATH_MULTIPLY: {
            bool leftIsNumber = node->left->type == NodeType::NUMBER;
            bool rightIsNumber = node->right->type == NodeType::NUMBER;
            //两个操作数中有一个是数字
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
            Node &u = node->left;
            Node &v = node->right;
            q.push(DiffNode(u.get(), true));
            Node u2 = Clone(u);
            Node v2 = Clone(v);
            q.push(DiffNode(v2.get(), false));
            node = Move(node) + Move(u2) * Move(v2);
            node->parent = parent;
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

Node Diff(const Node &node, const std::string &varname, int i) {
    Node node2 = Clone(node);
    return Diff(std::move(node2), varname, i);
}

Node Diff(Node &&node, const std::string &varname, int i) {
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

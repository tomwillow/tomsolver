#include "diff.h"

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

            //连接父级
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
        case MathOperator::MATH_TAN:
        case MathOperator::MATH_ARCSIN:
        case MathOperator::MATH_ARCCOS:
        case MathOperator::MATH_ARCTAN:
        case MathOperator::MATH_SQRT:
        case MathOperator::MATH_LOG:
        case MathOperator::MATH_LOG2:
        case MathOperator::MATH_LOG10:
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

            //连接父级
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
        case MathOperator::MATH_DIVIDE:
        case MathOperator::MATH_POWER:
            assert(0);
            return;

        case MathOperator::MATH_AND:
        case MathOperator::MATH_OR:
        case MathOperator::MATH_MOD:
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
//		case MATH_DIVIDE:
//			if (now->right->eType == NODE_NUMBER)// f(x)/number = f'(x)/number
//			{
//				Diff(now->left, var);
//			}
//			else
//			{
//				TNode *divide = now;
//				TNode *u1 = now->left;
//				TNode *v1 = now->right;
//
//				//创建减号
//				TNode *substract;
//				substract = NewNode(NODE_OPERATOR, MATH_SUB);
//
//				//创建2个乘号
//				TNode *multiply1, *multiply2;
//				multiply1 = NewNode(NODE_OPERATOR, MATH_MULTIPLY);
//				multiply2 = NewNode(NODE_OPERATOR, MATH_MULTIPLY);
//
//				//创建乘方
//				TNode *power;
//				power = NewNode(NODE_OPERATOR, MATH_POWER);
//
//				//连接除号下面2个节点：-, ^
//				divide->left = substract;
//				substract->parent = divide;
//				divide->right = power;
//				power->parent = divide;
//
//				//连接减号下面2个节点
//				substract->left = multiply1;
//				multiply1->parent = substract;
//				substract->right = multiply2;
//				multiply2->parent = substract;
//
//				//连接乘号1下面2个节点：u1, v1
//				multiply1->left = u1;
//				u1->parent = multiply1;
//				multiply1->right = v1;
//				v1->parent = multiply1;
//
//				//创建u2, v2
//				TNode *u2, *v2;
//				u2 = CopyNodeTree(u1);
//				v2 = CopyNodeTree(v1);
//
//				//连接乘号2下面的u2, v2
//				multiply2->left = u2;
//				u2->parent = multiply2;
//				multiply2->right = v2;
//				v2->parent = multiply2;
//
//				//创建v3, 2
//				TNode *v3, *num2;
//				v3 = CopyNodeTree(v1);
//				num2 = NewNode(NODE_NUMBER);
//				num2->value = 2;
//
//				//连接^下面的v3和2
//				power->left = v3;
//				v3->parent = power;
//				power->right = num2;
//				num2->parent = power;
//
//				Diff(u1, var);
//				Diff(v2, var);
//
//			}
//			return;
//		case MATH_POWER:
//		{
//			bool LChildIsNumber = now->left->eType == NODE_NUMBER;
//			bool RChildIsNumber = now->right->eType == NODE_NUMBER;
//			if (LChildIsNumber && RChildIsNumber)
//			{
//				delete now->left;
//				delete now->right;
//				now->left = NULL;
//				now->right = NULL;
//				now->eType = NODE_NUMBER;
//				now->eOperator = MATH_NULL;
//				now->value = 0.0;
//				return;
//			}
//
//			TNode *multiply1 = NewNode(NODE_OPERATOR, MATH_MULTIPLY);
//			TNode *multiply2 = NewNode(NODE_OPERATOR, MATH_MULTIPLY);
//			TNode *power = now;
//			TNode *u = now->left;
//			TNode *v = now->right;
//			TNode *u2 = CopyNodeTree(u);
//			TNode *v2 = CopyNodeTree(v);
//
//			if (power == head)
//			{
//				head = multiply1;
//			}
//			else
//			{
//				if (power->parent->left == power)
//					power->parent->left = multiply1;
//				if (power->parent->right == power)
//					power->parent->right = multiply1;
//				multiply1->parent = power->parent;
//			}
//
//			if (RChildIsNumber)
//				v->value -= 1.0;
//
//			multiply1->left = power;
//			power->parent = multiply1;
//
//			multiply1->right = multiply2;
//			multiply2->parent = multiply1;
//
//			multiply2->left = v2;
//			v2->parent = multiply2;
//
//			if (RChildIsNumber)
//			{
//				multiply2->right = u2;
//				u2->parent = multiply2;
//				Diff(u2, var);
//				return;
//			}
//			else
//			{
//				TNode *ln = NewNode(NODE_FUNCTION, MATH_LOG);
//
//				multiply2->right = ln;
//				ln->parent = multiply2;
//
//				ln->left = u2;
//				u2->parent = ln;
//
//				Diff(multiply2, var);
//				return;
//			}
//			return;
//		}
//		}
//		break;
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
    n->CheckParent();
#endif
    n->Simplify();
#ifndef NDEBUG
    n->CheckParent();
#endif
    return n;
}

} // namespace tomsolver

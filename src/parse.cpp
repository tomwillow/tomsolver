#include "parse.h"

#include "config.h"

#include <vector>
#include <cassert>
#include <regex>

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
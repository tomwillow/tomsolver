#include "parse.h"

#include "error_type.h"
#include "math_operator.h"
#include "node.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <functional>
#include <iterator>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace tomsolver {

namespace {

constexpr auto fnv1a(internal::StringView s) {
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

MathOperator Str2Function(internal::StringView s) noexcept {
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

std::deque<Token> ParseFunctions::ParseToTokens(StringView content) {

    if (content.empty()) {
        throw SingleParseError(0, 0, "empty input", content);
    }

    auto iter = content.begin(), nameIter = iter;
    std::deque<Token> ret;

    auto tryComfirmToken = [&ret, &iter, &nameIter, &content] {
        if (size_t size = std::distance(nameIter, iter)) {
            auto exp = StringView{&*nameIter, size};
            auto &token =
                ret.emplace_back(0, static_cast<int>(std::distance(content.begin(), nameIter)), false, exp, content);

            auto expStr = exp.toString();
            // 检验是否为浮点数
            try {
                std::size_t sz;
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
            if (!VarNameIsLegal(expStr)) // 变量名首字符需为下划线或字母
            {
                throw SingleParseError(token.line, token.pos, exp, "Invalid variable name: \"", exp, "\"");
            }

            token.node = Var(expStr);
        }
    };

    while (iter != content.end()) {
        if (IsBasicOperator(*iter)) {
            tryComfirmToken();
            auto unaryOp = ret.empty() || (ret.back().node->type == NodeType::OPERATOR &&
                                           ret.back().node->op != MathOperator::MATH_RIGHT_PARENTHESIS);
            ret.emplace_back(0, static_cast<int>(std::distance(content.begin(), iter)), true, StringView{&*iter, 1},
                             content)
                .node = Op(BaseOperatorCharToEnum(*iter, unaryOp));
            nameIter = ++iter;
        } else if (isspace(*iter)) {
            // 忽略tab (\t) whitespaces (\n, \v, \f, \r) space
            tryComfirmToken();
            nameIter = ++iter;
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

Node Parse(internal::StringView expression) {
    auto tokens = internal::ParseFunctions::ParseToTokens(expression);
    auto postOrder = internal::ParseFunctions::InOrderToPostOrder(tokens);
    auto node = internal::ParseFunctions::BuildExpressionTree(postOrder);
    return node;
}

Node operator""_f(const char *exp, size_t) {
    return Parse(exp);
}

} // namespace tomsolver
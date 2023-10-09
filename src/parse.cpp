#include "parse.h"

#include "config.h"

#include <vector>
#include <cassert>
#include <regex>

using std::string;
using std::to_string;

namespace tomsolver {

inline ParseError::ParseError(int line, int pos, const std::string &content, const std::string &errInfo)
    : std::runtime_error(""), line(line), pos(pos), content(content), errInfo(errInfo) {
    whatStr = "[Parse Error] " + errInfo + " at(" + to_string(line) + ", " + to_string(pos) + "):\n";
    whatStr += content + "\n";
    whatStr += string(pos, ' ') + "^---- error position";
}

const char *ParseError::what() const noexcept {
    return whatStr.c_str();
}

int ParseError::GetLine() const noexcept {
    return line;
}

int ParseError::GetPos() const noexcept {
    return pos;
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

struct Token {
    int line;
    int pos;
    bool isBaseOperator;
    std::string s;
    Token(int line, int pos, bool isBaseOperator, const std::string &s)
        : line(line), pos(pos), isBaseOperator(isBaseOperator), s(s) {}
};

// 粗切分：利用operator切分
std::vector<Token> SplitRough(const std::string &expression) {
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
                ret.push_back(Token(0, tempBeg, false, temp));
                tempBeg = i;

                temp.clear();
            }
            ret.push_back(Token(0, tempBeg, true, std::string{c}));
            tempBeg = i + 1;
        }
    }
    if (!temp.empty()) {
        ret.push_back(Token(0, tempBeg, false, temp));
        temp.clear();
    }

    return ret;
}

std::vector<Node> ParseToTokens(const std::string &expression) {

    if (expression.empty()) {
        throw ParseError(0, 0, "empty input", expression);
    }

    std::vector<Token> tokens = SplitRough(expression);

    std::vector<Node> ret;
    // 二次切分：切分出3类元素
    for (size_t i = 0; i < tokens.size(); i++) {
        const Token &token = tokens[i];
        auto &s = tokens[i].s;
        if (tokens[i].isBaseOperator) // 识别出基本运算符（括号也在其中）
        {
            Node tempNode = std::make_unique<NodeImpl>(NodeType::OPERATOR, BaseOperatorCharToEnum(s[0]), 0, "");
            ret.push_back(std::move(tempNode));
            continue;
        }

        // 检验是否为浮点数
        try {
            std::size_t sz;
            double d = std::stod(s, &sz);
            if (sz != s.size()) {
                throw std::invalid_argument("");
            }
            ret.push_back(Num(d));
            continue;
        } catch (const std::exception &) {}

        // 识别出函数
        MathOperator op = Str2Function(s);
        if (op != MathOperator::MATH_NULL) {
            Node tempNode = std::make_unique<NodeImpl>(NodeType::OPERATOR, op, 0, "");
            ret.push_back(std::move(tempNode));
            continue;
        }

        // 变量
        // 非运算符、数字、函数
        if (!VarNameIsLegal(s)) // 变量名首字符需为下划线或字母
        {
            throw ParseError(token.line, token.pos, expression, "Invalid variable name: \"" + s + "\"");
        }

        ret.push_back(Var(s));
    }
    // 此时3类元素均已切分

    // 识别取正运算符与取负运算符
    size_t i = 0;
    if (ret[0]->op == MathOperator::MATH_ADD) {
        ret[0]->op = MathOperator::MATH_POSITIVE;
        i++;
    }
    if (ret[0]->op == MathOperator::MATH_SUB) {
        ret[0]->op = MathOperator::MATH_NEGATIVE;
        i++;
    }
    for (; i < ret.size();) {
        if (ret[i]->type == NodeType::OPERATOR && ret[i]->op != MathOperator::MATH_RIGHT_PARENTHESIS) {
            if (i + 1 < ret.size())
                i++;
            else
                break;
            if (ret[i]->op == MathOperator::MATH_ADD) {
                ret[i]->op = MathOperator::MATH_POSITIVE;
                i++;
                continue;
            }
            if (ret[i]->op == MathOperator::MATH_SUB) {
                ret[i]->op = MathOperator::MATH_NEGATIVE;
                i++;
                continue;
            }
        } else
            i++;
    }

    return ret;
}
} // namespace internal
} // namespace tomsolver
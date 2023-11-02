#pragma once

#include "node.h"

#include <stdexcept>
#include <string_view>
#include <utility>

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
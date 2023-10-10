#pragma once

#include "node.h"

#include <stdexcept>

namespace tomsolver {

class ParseError : public std::runtime_error {
public:
    ParseError(int line, int pos, const std::string &content, const std::string &errInfo);

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
} // namespace tomsolver
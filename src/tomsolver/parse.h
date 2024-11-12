#pragma once

#include "node.h"

#include <cstring>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace tomsolver {

namespace internal {

class StringView {
public:
    constexpr StringView() noexcept = default;
    constexpr StringView(const char *str, size_t len) noexcept : str{str}, len{len} {}
    constexpr StringView(const char *str) noexcept : str{str} {
        while (*str++) {
            len++;
        }
    }
    StringView(const std::string &str) noexcept : StringView{str.data(), str.size()} {}
    constexpr StringView(const StringView &) noexcept = default;
    constexpr StringView &operator=(const StringView &) noexcept = default;

    constexpr auto begin() const noexcept {
        return str;
    }
    constexpr auto end() const noexcept {
        return str + len;
    }
    constexpr auto empty() const noexcept {
        return !len;
    }

    auto toString() const noexcept {
        return std::string{begin(), end()};
    }

    template <typename Stream>
    friend Stream &operator<<(Stream &s, const internal::StringView &sv) {
        s.rdbuf()->sputn(sv.str, sv.len);
        return s;
    }

private:
    const char *str = nullptr;
    size_t len = 0;
};

template <typename Stream>
void append(Stream &) {}

template <typename Stream, typename T, typename... Ts>
void append(Stream &s, T &&arg, Ts &&...args) {
    s << std::forward<T>(arg);
    append(s, std::forward<Ts>(args)...);
}

struct Token;
} // namespace internal

class ParseError : public std::runtime_error {
public:
protected:
    ParseError() : std::runtime_error("parse error") {}
};

class SingleParseError : public ParseError {
public:
    template <typename... T>
    SingleParseError(int line, int pos, internal::StringView content, T &&...errInfo)
        : line(line), pos(pos), content(content) {
        std::stringstream ss;

        ss << "[Parse Error] ";
        internal::append(ss, std::forward<T>(errInfo)...);
        ss << " at(" << line << ", " << pos << "):\n"
           << content << "\n"
           << std::string(pos, ' ') << "^---- error position";

        whatStr = ss.str();
    }

    virtual const char *what() const noexcept override;

    int GetLine() const noexcept;

    int GetPos() const noexcept;

private:
    int line;                     // the line index
    int pos;                      // the position of character
    internal::StringView content; // the whole content of the line
    std::string whatStr;          // the whole error message
};

class MultiParseError : public ParseError {
public:
    MultiParseError(const std::vector<SingleParseError> &parseErrors);

    virtual const char *what() const noexcept override;

private:
    std::vector<SingleParseError> parseErrors;
    std::string whatStr; // the whole error message
};

namespace internal {

struct Token {
    internal::StringView s;       // the string of this token
    int line;                     // the line index
    int pos;                      // the position of character
    bool isBaseOperator;          // if is base operator (single-character operator or parenthesis)
    internal::StringView content; // the whole content of the line
    Node node;                    // node
    Token(int line, int pos, bool isBaseOperator, StringView s, StringView content)
        : s(s), line(line), pos(pos), isBaseOperator(isBaseOperator), content(content) {}
};

class ParseFunctions {
public:
    /**
     * 解析表达式字符串为in order记号流。其实就是做词法分析。
     * @exception ParseError
     */
    static std::deque<Token> ParseToTokens(StringView expression);

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
Node Parse(internal::StringView expression);

Node operator""_f(const char *exp, size_t);

} // namespace tomsolver
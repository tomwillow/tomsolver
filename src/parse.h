#pragma once

#include "node.h"

#include <stdexcept>

namespace tomsolver {

class ParseError : public std::runtime_error {
public:
    ParseError(int line, int pos, const std::string &content, const std::string &errInfo);

    virtual const char *what() const noexcept override;

private:
    int line;
    int pos;
    std::string content;
    std::string errInfo;
    std::string whatStr;
};

namespace internal {

/**
 * 解析表达式字符串为记号流。
 * @exception ParseError
 */
std::vector<Node> ParseToTokens(const std::string &expression);

} // namespace internal
} // namespace tomsolver
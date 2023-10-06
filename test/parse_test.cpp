#include "parse.h"

#include "memory_leak_detection.h"

#include <gtest/gtest.h>

using namespace std;
using namespace tomsolver;

TEST(Parse, Base) {
    MemoryLeakDetection mld;

    {
        vector<Node> tokens = internal::ParseToTokens("1+2");
        ASSERT_TRUE(tokens[0]->Equal(Num(1)));
        ASSERT_TRUE(tokens[1]->Equal(internal::Operator(MathOperator::MATH_ADD)));
        ASSERT_TRUE(tokens[2]->Equal(Num(2)));
    }
}
TEST(Parse, IllegalChar) {
    MemoryLeakDetection mld;

    try {
        vector<Node> tokens = internal::ParseToTokens("1#+2");
        FAIL();
    } catch (const ParseError &err) { cout << err.what() << endl; }

    try {
        vector<Node> tokens = internal::ParseToTokens("a*cos(x1) + b*cos(x1-x2) + c*cos(?x1-x2-x3)");
        FAIL();
    } catch (const ParseError &err) { cout << err.what() << endl; }
}

TEST(Parse, PositiveNegative) {
    MemoryLeakDetection mld;

    {
        vector<Node> tokens = internal::ParseToTokens("1/+2");
        ASSERT_TRUE(tokens[0]->Equal(Num(1)));
        ASSERT_TRUE(tokens[1]->Equal(internal::Operator(MathOperator::MATH_DIVIDE)));
        ASSERT_TRUE(tokens[2]->Equal(internal::Operator(MathOperator::MATH_POSITIVE)));
        ASSERT_TRUE(tokens[3]->Equal(Num(2)));
    }

    {
        vector<Node> tokens = internal::ParseToTokens("1/-2");
        ASSERT_TRUE(tokens[0]->Equal(Num(1)));
        ASSERT_TRUE(tokens[1]->Equal(internal::Operator(MathOperator::MATH_DIVIDE)));
        ASSERT_TRUE(tokens[2]->Equal(internal::Operator(MathOperator::MATH_NEGATIVE)));
        ASSERT_TRUE(tokens[3]->Equal(Num(2)));
    }

    {
        vector<Node> tokens = internal::ParseToTokens("-1--2");
        ASSERT_TRUE(tokens[0]->Equal(internal::Operator(MathOperator::MATH_NEGATIVE)));
        ASSERT_TRUE(tokens[1]->Equal(Num(1)));
        ASSERT_TRUE(tokens[2]->Equal(internal::Operator(MathOperator::MATH_SUB)));
        ASSERT_TRUE(tokens[3]->Equal(internal::Operator(MathOperator::MATH_NEGATIVE)));
        ASSERT_TRUE(tokens[4]->Equal(Num(2)));
    }
}

TEST(Parse, Mix) {
    MemoryLeakDetection mld;

    { vector<Node> tokens = internal::ParseToTokens("a*cos(x1) + b*cos(x1-x2) + c*cos(x1-x2-x3)"); }
}
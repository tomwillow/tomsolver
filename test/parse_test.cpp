#include "parse.h"
#include "config.h"

#include "memory_leak_detection.h"

#include <gtest/gtest.h>

#include <random>

using namespace std;
using namespace tomsolver;

TEST(Parse, Base) {
    MemoryLeakDetection mld;

    {
        vector<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("1+2");
        ASSERT_TRUE(tokens[0].node->Equal(Num(1)));
        ASSERT_TRUE(tokens[1].node->Equal(internal::Operator(MathOperator::MATH_ADD)));
        ASSERT_TRUE(tokens[2].node->Equal(Num(2)));
    }
}

TEST(Parse, Number) {
    MemoryLeakDetection mld;

    {
        vector<internal::Token> tokens = internal::ParseFunctions::ParseToTokens(".12345");
        ASSERT_TRUE(tokens[0].node->Equal(Num(.12345)));
    }

    {
        vector<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("7891.123");
        ASSERT_TRUE(tokens[0].node->Equal(Num(7891.123)));
    }

    {
        vector<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("1e0");
        ASSERT_TRUE(tokens[0].node->Equal(Num(1e0)));
    }

    std::default_random_engine eng(
        static_cast<long>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));

    std::uniform_real_distribution<double> unif;

    for (int i = 0; i < 100; ++i) {
        double d = unif(eng);
        std::string expected = tomsolver::ToString(d);
        vector<internal::Token> tokens = internal::ParseFunctions::ParseToTokens(expected);
        ASSERT_EQ(expected, tokens[0].node->ToString());
    }
}

TEST(Parse, IllegalChar) {
    MemoryLeakDetection mld;

    try {
        vector<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("1#+2");
        FAIL();
    } catch (const ParseError &err) {
        cout << err.what() << endl;
        ASSERT_EQ(err.GetPos(), 0);
    }

    try {
        vector<internal::Token> tokens =
            internal::ParseFunctions::ParseToTokens("a*cos(x1) + b*cos(x1-x2) + c*cos(?x1-x2-x3)");
        FAIL();
    } catch (const ParseError &err) {
        cout << err.what() << endl;
        ASSERT_EQ(err.GetPos(), 33);
    }
}

TEST(Parse, PositiveNegative) {
    MemoryLeakDetection mld;

    {
        vector<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("1/+2");
        ASSERT_TRUE(tokens[0].node->Equal(Num(1)));
        ASSERT_TRUE(tokens[1].node->Equal(internal::Operator(MathOperator::MATH_DIVIDE)));
        ASSERT_TRUE(tokens[2].node->Equal(internal::Operator(MathOperator::MATH_POSITIVE)));
        ASSERT_TRUE(tokens[3].node->Equal(Num(2)));
    }

    {
        vector<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("1/-2");
        ASSERT_TRUE(tokens[0].node->Equal(Num(1)));
        ASSERT_TRUE(tokens[1].node->Equal(internal::Operator(MathOperator::MATH_DIVIDE)));
        ASSERT_TRUE(tokens[2].node->Equal(internal::Operator(MathOperator::MATH_NEGATIVE)));
        ASSERT_TRUE(tokens[3].node->Equal(Num(2)));
    }

    {
        vector<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("-1--2");
        ASSERT_TRUE(tokens[0].node->Equal(internal::Operator(MathOperator::MATH_NEGATIVE)));
        ASSERT_TRUE(tokens[1].node->Equal(Num(1)));
        ASSERT_TRUE(tokens[2].node->Equal(internal::Operator(MathOperator::MATH_SUB)));
        ASSERT_TRUE(tokens[3].node->Equal(internal::Operator(MathOperator::MATH_NEGATIVE)));
        ASSERT_TRUE(tokens[4].node->Equal(Num(2)));
    }
}

TEST(Parse, Mix) {
    MemoryLeakDetection mld;

    {
        vector<internal::Token> tokens =
            internal::ParseFunctions::ParseToTokens("a*cos(x1) + b*cos(x1-x2) + c*cos(x1-x2-x3)");
    }
}
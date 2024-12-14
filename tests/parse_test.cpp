#include <tomsolver/config.h>
#include <tomsolver/functions.h>
#include <tomsolver/parse.h>

#include "memory_leak_detection.h"

#include <gtest/gtest.h>

#include <random>

using namespace tomsolver;

using std::cout;
using std::deque;
using std::endl;

TEST(Parse, Base) {
    MemoryLeakDetection mld;
    std::setlocale(LC_ALL, ".UTF8");

    {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("1+2");
        ASSERT_TRUE(tokens[0].node->Equal(Num(1)));
        ASSERT_TRUE(tokens[1].node->Equal(internal::Operator(MathOperator::MATH_ADD)));
        ASSERT_TRUE(tokens[2].node->Equal(Num(2)));
    }
}

TEST(Parse, Number) {
    MemoryLeakDetection mld;

    {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens(".12345");
        ASSERT_TRUE(tokens[0].node->Equal(Num(.12345)));
    }

    {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("7891.123");
        ASSERT_TRUE(tokens[0].node->Equal(Num(7891.123)));
    }

    {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("1e0");
        ASSERT_TRUE(tokens[0].node->Equal(Num(1e0)));
    }

    std::default_random_engine eng(
        static_cast<long>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));

    std::uniform_real_distribution<double> unif;

    for (int i = 0; i < 100; ++i) {
        double d = unif(eng);
        std::string expected = tomsolver::ToString(d);
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens(expected);
        ASSERT_EQ(expected, tokens[0].node->ToString());
    }
}

TEST(Parse, IllegalChar) {
    MemoryLeakDetection mld;

    try {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("1#+2");
        FAIL();
    } catch (const SingleParseError &err) {
        cout << err.what() << endl;
        ASSERT_EQ(err.GetPos(), 0);
    }

    try {
        deque<internal::Token> tokens =
            internal::ParseFunctions::ParseToTokens("a*cos(x1) + b*cos(x1-x2) + c*cos(?x1-x2-x3)");
        FAIL();
    } catch (const SingleParseError &err) {
        cout << err.what() << endl;
        ASSERT_EQ(err.GetPos(), 33);
    }
}

TEST(Parse, PositiveNegative) {
    MemoryLeakDetection mld;

    {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("1/+2");
        ASSERT_TRUE(tokens[0].node->Equal(Num(1)));
        ASSERT_TRUE(tokens[1].node->Equal(internal::Operator(MathOperator::MATH_DIVIDE)));
        ASSERT_TRUE(tokens[2].node->Equal(internal::Operator(MathOperator::MATH_POSITIVE)));
        ASSERT_TRUE(tokens[3].node->Equal(Num(2)));
    }

    {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("1/-2");
        ASSERT_TRUE(tokens[0].node->Equal(Num(1)));
        ASSERT_TRUE(tokens[1].node->Equal(internal::Operator(MathOperator::MATH_DIVIDE)));
        ASSERT_TRUE(tokens[2].node->Equal(internal::Operator(MathOperator::MATH_NEGATIVE)));
        ASSERT_TRUE(tokens[3].node->Equal(Num(2)));
    }

    {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("-1--2");
        ASSERT_TRUE(tokens[0].node->Equal(internal::Operator(MathOperator::MATH_NEGATIVE)));
        ASSERT_TRUE(tokens[1].node->Equal(Num(1)));
        ASSERT_TRUE(tokens[2].node->Equal(internal::Operator(MathOperator::MATH_SUB)));
        ASSERT_TRUE(tokens[3].node->Equal(internal::Operator(MathOperator::MATH_NEGATIVE)));
        ASSERT_TRUE(tokens[4].node->Equal(Num(2)));
    }
}

TEST(Parse, PostOrder) {
    MemoryLeakDetection mld;

    {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("1*(2-3)");

        auto postOrder = internal::ParseFunctions::InOrderToPostOrder(tokens);

        ASSERT_TRUE(postOrder[0].node->Equal(Num(1)));
        ASSERT_TRUE(postOrder[1].node->Equal(Num(2)));
        ASSERT_TRUE(postOrder[2].node->Equal(Num(3)));
        ASSERT_TRUE(postOrder[3].node->Equal(internal::Operator(MathOperator::MATH_SUB)));
        ASSERT_TRUE(postOrder[4].node->Equal(internal::Operator(MathOperator::MATH_MULTIPLY)));
    }

    {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("1*2-3");

        auto postOrder = internal::ParseFunctions::InOrderToPostOrder(tokens);

        ASSERT_TRUE(postOrder[0].node->Equal(Num(1)));
        ASSERT_TRUE(postOrder[1].node->Equal(Num(2)));
        ASSERT_TRUE(postOrder[2].node->Equal(internal::Operator(MathOperator::MATH_MULTIPLY)));
        ASSERT_TRUE(postOrder[3].node->Equal(Num(3)));
        ASSERT_TRUE(postOrder[4].node->Equal(internal::Operator(MathOperator::MATH_SUB)));
    }

    {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("1-2-3");

        auto postOrder = internal::ParseFunctions::InOrderToPostOrder(tokens);

        ASSERT_TRUE(postOrder[0].node->Equal(Num(1)));
        ASSERT_TRUE(postOrder[1].node->Equal(Num(2)));
        ASSERT_TRUE(postOrder[2].node->Equal(internal::Operator(MathOperator::MATH_SUB)));
        ASSERT_TRUE(postOrder[3].node->Equal(Num(3)));
        ASSERT_TRUE(postOrder[4].node->Equal(internal::Operator(MathOperator::MATH_SUB)));
    }

    {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("2^3^4");

        auto postOrder = internal::ParseFunctions::InOrderToPostOrder(tokens);

        ASSERT_TRUE(postOrder[0].node->Equal(Num(2)));
        ASSERT_TRUE(postOrder[1].node->Equal(Num(3)));
        ASSERT_TRUE(postOrder[2].node->Equal(Num(4)));
        ASSERT_TRUE(postOrder[3].node->Equal(internal::Operator(MathOperator::MATH_POWER)));
        ASSERT_TRUE(postOrder[4].node->Equal(internal::Operator(MathOperator::MATH_POWER)));
    }
}

TEST(Parse, PostOrderError) {
    MemoryLeakDetection mld;

    {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("1*2-3)");

        try {
            auto postOrder = internal::ParseFunctions::InOrderToPostOrder(tokens);
            FAIL();
        } catch (const ParseError &err) {
            cout << err.what() << endl;
        }
    }

    {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("(1*2-3");

        try {
            auto postOrder = internal::ParseFunctions::InOrderToPostOrder(tokens);
            FAIL();
        } catch (const ParseError &err) {
            cout << err.what() << endl;
        }
    }
}

TEST(Parse, BuildTree) {
    MemoryLeakDetection mld;
    {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("1*(2-3)");
        auto postOrder = internal::ParseFunctions::InOrderToPostOrder(tokens);
        auto node = internal::ParseFunctions::BuildExpressionTree(postOrder);
        ASSERT_EQ(node->ToString(), "1*(2-3)");
        node->CheckParent();
    }

    {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("1*2-3");
        auto postOrder = internal::ParseFunctions::InOrderToPostOrder(tokens);
        auto node = internal::ParseFunctions::BuildExpressionTree(postOrder);
        ASSERT_EQ(node->ToString(), "1*2-3");
        node->CheckParent();
    }

    {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("x^2-y^2-7");
        auto postOrder = internal::ParseFunctions::InOrderToPostOrder(tokens);
        auto node = internal::ParseFunctions::BuildExpressionTree(postOrder);
        ASSERT_EQ(node->ToString(), "x^2-y^2-7");
        node->CheckParent();
    }
}

TEST(Parse, Mix) {
    MemoryLeakDetection mld;

    {
        deque<internal::Token> tokens =
            internal::ParseFunctions::ParseToTokens("a*cos(x1) + b*cos(x1-x2) + c*cos(x1-x2-x3)");
        auto postOrder = internal::ParseFunctions::InOrderToPostOrder(tokens);
        auto node = internal::ParseFunctions::BuildExpressionTree(postOrder);
        node->CheckParent();

        Node expected = Var("a") * cos(Var("x1")) + Var("b") * cos(Var("x1") - Var("x2")) +
                        Var("c") * cos(Var("x1") - Var("x2") - Var("x3"));
        ASSERT_TRUE(node->Equal(expected));
    }

    try {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("x(1)*cos(2)");
        auto postOrder = internal::ParseFunctions::InOrderToPostOrder(tokens);
        auto node = internal::ParseFunctions::BuildExpressionTree(postOrder);
        FAIL();
    } catch (const ParseError &err) {
        cout << err.what() << endl;
    }

    try {
        deque<internal::Token> tokens =
            internal::ParseFunctions::ParseToTokens("x(1)*cos(x(2)) + x(2)*sin(x(1)) - 0.5");
        auto postOrder = internal::ParseFunctions::InOrderToPostOrder(tokens);
        auto node = internal::ParseFunctions::BuildExpressionTree(postOrder);
        FAIL();
    } catch (const ParseError &err) {
        cout << err.what() << endl;
    }
}
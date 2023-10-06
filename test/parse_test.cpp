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
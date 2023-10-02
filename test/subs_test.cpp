#include "tomsolver.h"

#include "memory_leak_detection.h"

#include <gtest/gtest.h>

#include <random>

using namespace std;
using namespace tomsolver;

TEST(Subs, Base) {
    MemoryLeakDetection mld;

    Node n = Var("x");
    ASSERT_EQ(Subs(n, "x", Var("y"))->ToString(), "y");

    ASSERT_EQ(Subs(n, "x", Num(100))->ToString(), "100");

    ASSERT_DOUBLE_EQ(Subs(std::move(n), "x", Num(99))->Vpa(), 99.0);
    ASSERT_EQ(n, nullptr);
}

TEST(Subs, Combine) {
    MemoryLeakDetection mld;

    {
        // x*y+sin(x)
        Node n = Var("x") * Var("y") + sin(Var("x"));

        n = Subs(std::move(n), "x", Var("x") + Num(1));

        ASSERT_EQ(n->ToString(), "(x+1)*y+sin(x+1)");
    }

    {
        // r*sin(x+y)
        Node n = Var("r") * sin(Var("x") + Var("y"));

        // -> 100*sin(2*pi+pi/6) == 50
        n = Subs(std::move(n), "x", Num(2.0 * PI));
        n = Subs(std::move(n), "y", Num(PI / 6.0));
        n = Subs(std::move(n), "r", Num(100));

        ASSERT_DOUBLE_EQ(n->Vpa(), 50.0);
    }
}
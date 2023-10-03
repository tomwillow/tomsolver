#include "subs.h"
#include "functions.h"

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

        // -> 100*sin(360deg+30deg) == 50
        n = Subs(std::move(n), "x", Num(radians(360.0)));
        n = Subs(std::move(n), "y", Num(radians(30.0)));
        n = Subs(std::move(n), "r", Num(100));

        ASSERT_DOUBLE_EQ(n->Vpa(), 50.0);
    }
}

TEST(Subs, Multiple) {
    MemoryLeakDetection mld;

    {
        // x*y+sin(x)
        Node n = Var("x") * Var("y") + sin(Var("x"));

        // 交换x y
        n = Subs(std::move(n), {"x", "y"}, {Var("y"), Var("x")});
        ASSERT_EQ(n->ToString(), "y*x+sin(y)");

        // x -> cos(y)
        n = Subs(std::move(n), {"x"}, {cos(Var("y"))});
        ASSERT_EQ(n->ToString(), "y*cos(y)+sin(y)");
    }
}
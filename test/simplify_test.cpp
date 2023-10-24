#include "simplify.h"
#include "functions.h"

#include "helper.h"
#include "memory_leak_detection.h"

#include <gtest/gtest.h>

#include <random>

using namespace tomsolver;

using std::cout;
using std::endl;

TEST(Simplify, Base) {
    MemoryLeakDetection mld;

    Node n = sin(Num(0));
    Simplify(n);

    ASSERT_EQ(n->ToString(), "0");

    Node n2 = Num(1) + Num(2) * Num(3);
    Simplify(n2);

    ASSERT_EQ(n2->ToString(), "7");

    ASSERT_TRUE(n2->Equal(Num(7)));
}

TEST(Simplify, Add) {
    MemoryLeakDetection mld;

    {
        Node n = Var("x") + Num(0);
        Simplify(n);
        ASSERT_EQ(n->ToString(), "x");
        n->CheckParent();
    }

    {
        Node n = Num(0) + Var("x");
        Simplify(n);
        ASSERT_EQ(n->ToString(), "x");
        n->CheckParent();
    }
}

TEST(Simplify, Multiply) {
    MemoryLeakDetection mld;

    {
        Node n = Var("x") * Num(1) * Var("y") * Var("z");
        Simplify(n);
        ASSERT_EQ(n->ToString(), "x*y*z");
        n->CheckParent();
    }

    {
        Node n = cos(Var("x")) * Num(1);
        Simplify(n);
        ASSERT_EQ(n->ToString(), "cos(x)");
        n->CheckParent();
    }

    {
        Node n = Num(1) * Var("x") * Num(0) + Num(0) * Var("y");
        Simplify(n);
        ASSERT_EQ(n->ToString(), "0");
        n->CheckParent();
    }
}

TEST(Simplify, DoNotStackOverFlow) {
    MemoryLeakDetection mld;

    // 构造一个随机的长表达式
    auto pr = CreateRandomExpresionTree(100000);
    Node &node = pr.first;

    Simplify(node);
}
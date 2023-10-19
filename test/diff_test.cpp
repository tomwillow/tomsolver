#include "diff.h"
#include "functions.h"

#include "memory_leak_detection.h"

#include <gtest/gtest.h>

#include <random>

using namespace tomsolver;

using std::cout;
using std::endl;

TEST(Diff, Base) {
    MemoryLeakDetection mld;

    Node n = Var("a");
    ASSERT_TRUE(Diff(n, "a")->Equal(Num(1)));
    ASSERT_TRUE(Diff(n, "b")->Equal(Num(0)));

    ASSERT_TRUE(Diff(Num(1), "a")->Equal(Num(0)));

    // diff(a+b, a) == 1
    Node n2 = n + Var("b");
    ASSERT_TRUE(Diff(n2, "a")->Equal(Num(1)));
}

TEST(Diff, Negative) {
    MemoryLeakDetection mld;

    Node n = -Var("a");
    auto dn = Diff(n, "a");
    ASSERT_TRUE(dn->Equal(Num(-1)));
    ASSERT_TRUE(Diff(n, "b")->Equal(Num(0)));

    ASSERT_TRUE(Diff(-Num(1), "a")->Equal(Num(0)));

    // diff(-a+ -b, a) == -1
    Node n2 = n + -Var("b");
    auto dn2a = Diff(n2, "a");
    ASSERT_TRUE(dn2a->Equal(Num(-1)));
    auto dn2b = Diff(n2, "b");
    ASSERT_TRUE(dn2b->Equal(Num(-1)));

    // diff(-a+ +b, a) == -1
    Node n3 = n + +Var("b");
    ASSERT_TRUE(Diff(n3, "a")->Equal(Num(-1)));
    ASSERT_TRUE(Diff(n3, "b")->Equal(Num(1)));
}

TEST(Diff, Sin) {
    MemoryLeakDetection mld;

    {
        // sin'x = cos x
        Node n = sin(Var("x"));
        Node dn = Diff(n, "x");
        dn->CheckParent();
        cout << dn->ToString() << endl;
        ASSERT_TRUE(dn->Equal(cos(Var("x"))));
    }
}

TEST(Diff, Cos) {
    MemoryLeakDetection mld;

    {
        // cos'x = -sin x
        Node n = cos(Var("x"));
        Node dn = Diff(n, "x");
        dn->CheckParent();
        cout << dn->ToString() << endl;
        ASSERT_TRUE(dn->Equal(-sin(Var("x"))));
    }
}

TEST(Diff, Exp) {
    MemoryLeakDetection mld;

    {
        // (e^x)' = e^x
        Node n = exp(Var("x"));
        Node dn = Diff(n, "x");
        dn->CheckParent();
        cout << dn->ToString() << endl;
        ASSERT_TRUE(dn->Equal(exp(Var("x"))));
    }

    {
        // (e^sin(x))' = e^sin(x)*cos(x)
        Node n = exp(sin(Var("x")));
        Node dn = Diff(n, "x");
        dn->CheckParent();
        cout << dn->ToString() << endl;
        ASSERT_TRUE(dn->Equal(exp(sin(Var("x"))) * cos(Var("x"))));
    }
}

TEST(Diff, Multiply) {
    MemoryLeakDetection mld;

    // diff(5*a, a) == 5
    ASSERT_TRUE(Diff(Num(5) * Var("a"), "a")->Equal(Num(5)));

    // diff(b*5, b) == 5
    ASSERT_TRUE(Diff(Var("b") * Num(5), "b")->Equal(Num(5)));

    {
        // diff(a*b, a) == b
        Node n = Var("a") * Var("b");
        Node dn = Diff(n, "a");
        dn->CheckParent();
        cout << dn->ToString() << endl;
        ASSERT_TRUE(dn->Equal(Var("b")));
    }

    {
        // diff(a*b*a, a) ==
        Node n = Var("a") * Var("b") * Var("a");
        Node dn = Diff(n, "a");
        dn->CheckParent();
        cout << dn->ToString() << endl;
    }
}

TEST(Diff, Combine) {
    MemoryLeakDetection mld;

    {
        // diff(sin(a*b+c)*1*a, a)
        Node n = sin(Var("a") * Var("b") + Var("c")) * Num(1) * Var("a");
        Node dn = Diff(n, "a");
        dn->CheckParent();
        cout << dn->ToString() << endl;
        ASSERT_EQ(dn->ToString(), "cos(a*b+c)*b*a+sin(a*b+c)");
    }

    {
        // diff(sin(cos(x)+sin(x)), x) =
        Node n = sin(cos(Var("x")) + sin(Var("x")));
        Node dn = Diff(n, "x");
        dn->CheckParent();
        cout << dn->ToString() << endl;
        ASSERT_EQ(dn->ToString(), "cos(cos(x)+sin(x))*(-(sin(x))+cos(x))");
    }
}
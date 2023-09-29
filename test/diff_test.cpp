#include "tomsolver.h"

#include "memory_leak_detection.h"

#include <gtest/gtest.h>

#include <random>

using namespace std;
using namespace tomsolver;

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
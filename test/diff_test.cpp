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
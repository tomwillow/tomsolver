#include "tomsolver.h"

#include "memory_leak_detection.h"

#include <gtest/gtest.h>

#include <random>

using namespace std;
using namespace tomsolver;

TEST(Simplify, Base) {
    MemoryLeakDetection mld;

    Node n = sin(Num(0));
    n->Simplify();

    ASSERT_EQ(n->ToString(), "0");

    Node n2 = Num(1) + Num(2) * Num(3);
    n2->Simplify();

    ASSERT_EQ(n2->ToString(), "7");
}
#include "functions.h"

#include "memory_leak_detection.h"

#include <gtest/gtest.h>

#include <random>

using namespace std;
using namespace tomsolver;

TEST(Power, Base) {
    MemoryLeakDetection mld;

    Node n = Num(2) ^ Num(3);
    ASSERT_EQ(n->ToString(), "2^3");
    ASSERT_DOUBLE_EQ(n->Vpa(), 8);

    // CATION!
    Node n2 = Num(2) ^ Num(3) ^ Num(2);
    ASSERT_EQ(n2->ToString(), "(2^3)^2");
    ASSERT_DOUBLE_EQ(n2->Vpa(), 64);

    // CATION!
    Node n3 = Num(2) ^ (Num(3) ^ Num(2));
    ASSERT_EQ(n3->ToString(), "2^(3^2)");
    ASSERT_DOUBLE_EQ(n3->Vpa(), 512);
}
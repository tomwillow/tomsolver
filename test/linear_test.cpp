#include "linear.h"

#include "memory_leak_detection.h"

#include <gtest/gtest.h>

using namespace std;
using namespace tomsolver;

TEST(Linear, Base) {
    MemoryLeakDetection mld;

    Mat A = {{2, 1, -5, 1}, {1, -5, 0, 7}, {0, 2, 1, -1}, {1, 6, -1, -4}};
    Vec b = {13, -9, 6, 0};

    auto x = SolveLinear(A, b);

    Vec expected = {-66.5555555555555429, 25.6666666666666643, -18.777777777777775, 26.55555555555555};

    ASSERT_EQ(x, expected);
}
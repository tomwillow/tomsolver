#include "mat.h"

#include "memory_leak_detection.h"

#include <gtest/gtest.h>

using namespace std;
using namespace tomsolver;

TEST(Mat, Inverse) {
    MemoryLeakDetection mld;

    {
        Mat A = {{1, 2}, {3, 4}};
        auto inv = A.Inverse();
        Mat expected = {{-2, 1}, {1.5, -0.5}};
        ASSERT_EQ(inv, expected);
    }
    {
        Mat A = {{1, 2, 3}, {4, 5, 6}, {-2, 7, 8}};
        auto inv = A.Inverse();
        Mat expected = {{-0.083333333333333, 0.208333333333333, -0.125000000000000},
                        {-1.833333333333333, 0.583333333333333, 0.250000000000000},
                        {1.583333333333333, -0.458333333333333, -0.125000000000000}};
        ASSERT_EQ(inv, expected);
    }
    {
        Mat A = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
        try {
            auto inv = A.Inverse();
            FAIL();
        } catch (const MathError &e) { cout << "[Expected]" << e.what() << endl; }
    }
}

TEST(Mat, PositiveDetermine) {
    MemoryLeakDetection mld;

    {
        Mat A = {{1, 1, 1, 1}, {1, 2, 3, 4}, {1, 3, 6, 10}, {1, 4, 10, 20}};
        ASSERT_TRUE(A.PositiveDetermine());
    }
    {
        Mat A = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
        ASSERT_TRUE(!A.PositiveDetermine());
    }
}

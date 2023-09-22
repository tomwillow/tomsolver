#include "node.h"
#include "functions.h"

#include "memory_leak_detection.h"

#include <gtest/gtest.h>

#include <random>

using namespace std;
using namespace tomsolver;

TEST(Function, Sin)
{
    MemoryLeakDetection mld;

    ASSERT_DOUBLE_EQ(sin(Num(0))->Vpa(), 0);
    ASSERT_DOUBLE_EQ(sin(Num(PI/2.0))->Vpa(), 1.0);
}
#include "node.h"
#include "functions.h"

#include "memory_leak_detection.h"

#include <gtest/gtest.h>

#include <random>

using namespace std;
using namespace tomsolver;

TEST(Function, Trigonometric) {
    MemoryLeakDetection mld;

    int count = 100;

    auto seed = static_cast<unsigned int>(chrono::high_resolution_clock::now().time_since_epoch().count());
    cout << "seed = " << seed << endl;
    default_random_engine eng(seed);
    uniform_real_distribution<double> unifNum;

    for (int i = 0; i < count; ++i) {
        double num = unifNum(eng);
        ASSERT_DOUBLE_EQ(sin(Num(num))->Vpa(), sin(num));
        ASSERT_DOUBLE_EQ(cos(Num(num))->Vpa(), cos(num));
        ASSERT_DOUBLE_EQ(tan(Num(num))->Vpa(), tan(num));
        ASSERT_DOUBLE_EQ(asin(Num(num))->Vpa(), asin(num));
        ASSERT_DOUBLE_EQ(acos(Num(num))->Vpa(), acos(num));
        ASSERT_DOUBLE_EQ(atan(Num(num))->Vpa(), atan(num));
        ASSERT_DOUBLE_EQ(sqrt(Num(num))->Vpa(), sqrt(num));
        ASSERT_DOUBLE_EQ(log(Num(num))->Vpa(), log(num));
        ASSERT_DOUBLE_EQ(log2(Num(num))->Vpa(), log2(num));
        ASSERT_DOUBLE_EQ(log10(Num(num))->Vpa(), log10(num));
        ASSERT_DOUBLE_EQ(exp(Num(num))->Vpa(), exp(num));
    }
}

TEST(Function, ToString) {
    MemoryLeakDetection mld;

    Node f = Var("r") * sin(Var("omega") / Num(2.0) + Var("phi")) + Var("c");

    ASSERT_EQ(f->ToString(), "r*sin(omega/2+phi)+c");
}
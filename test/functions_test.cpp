#include "node.h"
#include "functions.h"
#include "config.h"

#include "memory_leak_detection.h"

#include <gtest/gtest.h>

#include <random>

#undef max
#undef min

using namespace tomsolver;

using std::cout;
using std::endl;

TEST(Function, Trigonometric) {
    MemoryLeakDetection mld;

    int count = 100;

    auto seed = static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    cout << "seed = " << seed << endl;
    std::default_random_engine eng(seed);
    std::uniform_real_distribution<double> unifNum;

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

TEST(Function, InvalidNumber) {
    MemoryLeakDetection mld;
    double inf = std::numeric_limits<double>::infinity();
    double inf2 = -std::numeric_limits<double>::infinity();
    double nan = std::numeric_limits<double>::quiet_NaN();
    double dblMax = std::numeric_limits<double>::max();

#define MY_ASSERT_THROW(statement, shouldThrow)                                                                        \
    if (shouldThrow) {                                                                                                 \
        try {                                                                                                          \
            statement;                                                                                                 \
            FAIL();                                                                                                    \
        } catch (const MathError &err) {                                                                               \
            std::cerr << "[Expected Exception]" << err.what() << std::endl;                                            \
        }                                                                                                              \
    } else {                                                                                                           \
        try {                                                                                                          \
            statement;                                                                                                 \
        } catch (const MathError &err) {                                                                               \
            std::cerr << "[Unexpected Exception]" << err.what() << std::endl;                                          \
            FAIL();                                                                                                    \
        }                                                                                                              \
    }

    auto Test = [&](bool shouldThrow) {
        MY_ASSERT_THROW((Num(inf) + Num(1))->Vpa(), shouldThrow);
        MY_ASSERT_THROW((Num(inf2) + Num(1))->Vpa(), shouldThrow);
        MY_ASSERT_THROW((Num(nan) + Num(1))->Vpa(), shouldThrow);

        MY_ASSERT_THROW((Num(1) / Num(0))->Vpa(), shouldThrow);

        // pow(DBL_DOUELB, 2)
        MY_ASSERT_THROW((Num(dblMax) ^ Num(2))->Vpa(), shouldThrow);

        MY_ASSERT_THROW(asin(Num(1.1))->Vpa(), shouldThrow);
        MY_ASSERT_THROW(asin(Num(-1.1))->Vpa(), shouldThrow);

        MY_ASSERT_THROW(acos(Num(1.1))->Vpa(), shouldThrow);
        MY_ASSERT_THROW(acos(Num(-1.1))->Vpa(), shouldThrow);

        MY_ASSERT_THROW(sqrt(Num(-0.1))->Vpa(), shouldThrow);

        MY_ASSERT_THROW(log(Num(0))->Vpa(), shouldThrow);
        MY_ASSERT_THROW(log2(Num(0))->Vpa(), shouldThrow);
        MY_ASSERT_THROW(log10(Num(0))->Vpa(), shouldThrow);
    };

    // 默认配置下，无效值应该抛异常
    Test(true);

    // 手动关闭无效值检查，不应抛异常
    {
        GetConfig().throwOnInvalidValue = false;

        Test(false);

        // 恢复配置
        GetConfig().Reset();
    }
}

TEST(Function, ToString) {
    MemoryLeakDetection mld;

    Node f = Var("r") * sin(Var("omega") / Num(2.0) + Var("phi")) + Var("c");

    ASSERT_EQ(f->ToString(), "r*sin(omega/2+phi)+c");
}
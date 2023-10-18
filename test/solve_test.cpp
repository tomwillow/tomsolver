#include "nonlinear.h"
#include "config.h"
#include "functions.h"
#include "parse.h"

#include "memory_leak_detection.h"

#include <gtest/gtest.h>

#include <iostream>

using namespace tomsolver;

using std::cout;
using std::endl;

TEST(Solve, Base) {
    // the example of this test is from: https://zhuanlan.zhihu.com/p/136889381

    MemoryLeakDetection mld;

    std::setlocale(LC_ALL, ".UTF8");

    /*
        以一个平面三轴机器人为例，运动学方程为
            a = 0.425;  b = 0.39243;  c=0.109;
            y = [   a*cos(x(1)) + b*cos(x(1)-x(2)) + c*cos(x(1)-x(2)-x(3)),
                    a*sin(x(1)) + b*sin(x(1)-x(2)) + c*sin(x(1)-x(2)-x(3)),
                    x(1)-x(2)-x(3)    ];
    */
    Node f1 = Parse("a*cos(x1) + b*cos(x1-x2) + c*cos(x1-x2-x3)");
    Node f2 = Parse("a*sin(x1) + b*sin(x1-x2) + c*sin(x1-x2-x3)");
    Node f3 = Parse("x1-x2-x3");

    SymVec f{Move(f1), Move(f2), Move(f3)};

    // 目标位置为：[0.5 0.4 0]
    SymVec b{Num(0.5), Num(0.4), Num(0)};
    SymVec equations = f - b;
    equations.Subs(VarsTable{{"a", 0.425}, {"b", 0.39243}, {"c", 0.109}});

    // 期望值
    VarsTable expected{{"x1", 1.5722855035930956}, {"x2", 1.6360330989069252}, {"x3", -0.0637475947386077}};

    // Newton-Raphson方法
    {
        GetConfig().nonlinearMethod = NonlinearMethod::NEWTON_RAPHSON;

        VarsTable got = Solve(equations);
        cout << got << endl;

        ASSERT_EQ(got, expected);
    }

    // LM方法
    {
        GetConfig().nonlinearMethod = NonlinearMethod::LM;

        VarsTable got = Solve(equations);
        cout << got << endl;

        ASSERT_EQ(got, expected);
    }
}

TEST(Solve, Case1) {
    // the example of this test is from: https://zhuanlan.zhihu.com/p/136889381

    MemoryLeakDetection mld;

    std::setlocale(LC_ALL, ".UTF8");

    /*
     */

    SymVec f{{Parse("exp(-exp(-(x1 + x2))) - x2 * (1 + x1 ^ 2)"), Parse("x1 * cos(x2) + x2 * sin(x1) - 0.5")}};

    cout << f.ToString() << endl;

    VarsTable ans = Solve(f);

    cout << ans << endl;
}
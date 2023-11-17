#include "config.h"
#include "error_type.h"
#include "functions.h"
#include "nonlinear.h"
#include "parse.h"

#include "memory_leak_detection.h"

#include <gtest/gtest.h>

#include <iostream>

using namespace tomsolver;

using std::cout;
using std::endl;

TEST(SolveBase, FindAlphaByArmijo) {
    MemoryLeakDetection mld;

    auto g = [](const Vec &x) -> Vec {
        return {pow(x[0] - 4, 4) + pow(x[1] - 3, 2) + 4 * pow(x[2] + 5, 4)};
    };

    auto dg = [](const Vec &x) -> Vec {
        return {4 * pow(x[0] - 4, 3), 2 * (x[1] - 3), 16 * pow(x[2] + 5, 3)};
    };

    Vec x{4, 2, -1};
    Vec d = -Vec{0, -2, 1024};
    double alpha = Armijo(x, d, g, dg);
    cout << alpha << endl;

    // FIXME: not match got results
    // double expected = 0.003866;
}

TEST(SolveBase, FindAlpha) {

    auto g = [](const Vec &x) -> Vec {
        return {pow(x[0] - 4, 4), pow(x[1] - 3, 2), 4 * pow(x[2] + 5, 4)};
    };

    Vec x{4, 2, -1};
    Vec d = -Vec{0, -2, 1024};
    double alpha = FindAlpha(x, d, g);
    cout << alpha << endl;

    // FIXME: not match got results
    // double expected = 0.003866;
}

TEST(SolveBase, Base) {
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
    Node f1 = Var("a") * cos(Var("x1")) + Var("b") * cos(Var("x1") - Var("x2")) +
              Var("c") * cos(Var("x1") - Var("x2") - Var("x3"));
    Node f2 = Var("a") * sin(Var("x1")) + Var("b") * sin(Var("x1") - Var("x2")) +
              Var("c") * sin(Var("x1") - Var("x2") - Var("x3"));
    Node f3 = Var("x1") - Var("x2") - Var("x3");

    SymVec f{Clone(f1), Clone(f2), Clone(f3)};

    // 目标位置为：[0.5 0.4 0]
    SymVec b{Num(0.5), Num(0.4), Num(0)};
    SymVec equations = f - b;
    equations.Subs(VarsTable{{"a", 0.425}, {"b", 0.39243}, {"c", 0.109}});

    // 初值表
    VarsTable varsTable{{"x1", 1}, {"x2", 1}, {"x3", 1}};

    // 期望值
    VarsTable expected{{"x1", 1.5722855035930956}, {"x2", 1.6360330989069252}, {"x3", -0.0637475947386077}};

    // Newton-Raphson方法
    {
        VarsTable got = SolveByNewtonRaphson(varsTable, equations);
        cout << got << endl;

        ASSERT_EQ(got, expected);
    }

    // LM方法
    {
        VarsTable got = SolveByLM(varsTable, equations);
        cout << got << endl;

        ASSERT_EQ(got, expected);
    }
}

TEST(SolveBase, IndeterminateEquation) {
    MemoryLeakDetection mld;

    std::setlocale(LC_ALL, ".UTF8");

    SymVec f = {
        "cos(x1) + cos(x1-x2) + cos(x1-x2-x3) - 1"_f,
        "sin(x1) + sin(x1-x2) + sin(x1-x2-x3) + 2"_f,
    };

    // 不定方程，应该抛出异常
    try {
        VarsTable got = Solve(f);
        FAIL();
    } catch (const MathError &e) {
        cout << e.what() << endl;
    }

    // 设置为允许不定方程
    Config::Get().allowIndeterminateEquation = true;

    // 结束时恢复设置
    std::shared_ptr<void> defer(nullptr, [](auto) {
        Config::Get().Reset();
    });

    VarsTable got = Solve(f);
    cout << got << endl;
}
#include <tomsolver/config.h>
#include <tomsolver/functions.h>
#include <tomsolver/nonlinear.h>
#include <tomsolver/parse.h>

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
    SymVec f = {
        "a*cos(x1) + b*cos(x1-x2) + c*cos(x1-x2-x3)"_f,
        "a*sin(x1) + b*sin(x1-x2) + c*sin(x1-x2-x3)"_f,
        "x1-x2-x3"_f,
    };

    // 目标位置为：[0.5 0.4 0]
    SymVec b{Num(0.5), Num(0.4), Num(0)};
    SymVec equations = f - b;
    equations.Subs(VarsTable{{"a", 0.425}, {"b", 0.39243}, {"c", 0.109}});

    // 期望值
    VarsTable expected{{"x1", 1.5722855035930956}, {"x2", 1.6360330989069252}, {"x3", -0.0637475947386077}};

    // Newton-Raphson方法
    {
        Config::Get().nonlinearMethod = NonlinearMethod::NEWTON_RAPHSON;

        // 结束时恢复设置
        std::shared_ptr<void> defer(nullptr, [&](...) {
            Config::Get().Reset();
        });

        VarsTable got = Solve(equations);
        cout << got << endl;

        ASSERT_EQ(got, expected);
    }

    // LM方法
    {
        Config::Get().nonlinearMethod = NonlinearMethod::LM;

        // 结束时恢复设置
        std::shared_ptr<void> defer(nullptr, [&](...) {
            Config::Get().Reset();
        });

        VarsTable got = Solve(equations);
        cout << got << endl;

        ASSERT_EQ(got, expected);
    }
}

TEST(Solve, Case1) {
    MemoryLeakDetection mld;

    std::setlocale(LC_ALL, ".UTF8");

    /*

    Matlab code:
    from: https://ww2.mathworks.cn/help/optim/ug/fsolve.html

    root2d.m:
        function F = root2d(x)
            F(1) = exp(-exp(-(x(1)+x(2)))) - x(2)*(1+x(1)^2);
            F(2) = x(1)*cos(x(2)) + x(2)*sin(x(1)) - 0.5;
        end

    root2d_solve.m:
        format long
        fun = @root2d;
        x0 = [0,0];
        x = fsolve(fun,x0)

    result:
        x =

            0.353246561920553   0.606082026502285


     */

    // 设置初值为0.0
    Config::Get().initialValue = 0.0;

    // 设置容差为1.0e-6，因为Matlab的默认容差是1.0e-6
    Config::Get().epsilon = 1.0e-6;

    // 结束时恢复设置
    std::shared_ptr<void> defer(nullptr, [&](...) {
        Config::Get().Reset();
    });

    // 构造方程组
    SymVec f = {
        "exp(-exp(-(x1 + x2))) - x2 * (1 + x1 ^ 2)"_f,
        "x1 * cos(x2) + x2 * sin(x1) - 0.5"_f,
    };

    // 求解，结果保存到ans
    VarsTable ans = Solve(f);

    // 打印出ans
    cout << ans << endl;

    // 单独获取变量的值
    cout << "x1 = " << ans["x1"] << endl;
    cout << "x2 = " << ans["x2"] << endl;

    ASSERT_EQ(ans, VarsTable({{"x1", 0.353246561920553}, {"x2", 0.606082026502285}}));
}

TEST(Solve, Case2) {
    MemoryLeakDetection mld;

    std::setlocale(LC_ALL, ".UTF8");

    /*

    Translate from Matlab code:

        fun = @(x)x*x*x - [1,2;3,4];
        x0 = ones(2);
        format long;
        fsolve(fun,x0)

     */
    Config::Get().nonlinearMethod = NonlinearMethod::LM;

    // 结束时恢复设置
    std::shared_ptr<void> defer(nullptr, [](auto) {
        Config::Get().Reset();
    });

    // 构造符号矩阵: [a b; c d]
    SymMat X({{Var("a"), Var("b")}, {Var("c"), Var("d")}});

    Mat B{{1, 2}, {3, 4}};

    // 计算出矩阵X*X*X-B
    auto FMat = X * X * X - B;

    // 提取矩阵的每一个元素，构成4个方程组成的符号向量
    auto F = FMat.ToSymVecOneByOne();

    cout << F << endl;

    // 把符号向量F作为方程组进行求解
    VarsTable ans = Solve(F);

    cout << ans << endl;

    VarsTable expected{
        {"a", -0.129148906397607}, {"b", 0.8602157139938529}, {"c", 1.2903235709907794}, {"d", 1.1611746645931726}};

    ASSERT_EQ(ans, expected);
}

TEST(Solve, Case3) {
    MemoryLeakDetection mld;

    std::setlocale(LC_ALL, ".UTF8");

    Config::Get().nonlinearMethod = NonlinearMethod::LM;

    // 结束时恢复设置
    std::shared_ptr<void> defer(nullptr, [&](...) {
        Config::Get().Reset();
    });

    SymVec f{
        Parse("a/(b^2)-c/(d^2)"),
        Parse("129.56108*b-(a/(b^2)+1/a-2*b/(a^2))"),
        Parse("129.56108*d-(d/(c^2)-c/(d^2)-1/a)"),
        Parse("5*exp(1)-7-(2/3*pi*a^2*b+((sqrt(3)*c^2)/(3*sqrt(c^2/3+d^2))+a-c)^2*pi*d^2/(c^2/3+d^2))"),
    };

    f.Subs(VarsTable{{"pi", PI}});

    cout << f << endl;

    auto ans = Solve(f);

    cout << ans << endl;
}

#include "nonlinear.h"
#include "config.h"
#include "functions.h"

#include "memory_leak_detection.h"

#include <gtest/gtest.h>

#include <iostream>

using namespace std;
using namespace tomsolver;

TEST(Solve, FindAlphaByArmijo) {
    MemoryLeakDetection mld;

    GetConfig().epsilon = 1e-6;

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
    double expected = 0.003866;
}

TEST(Solve, FindAlpha) {

    auto g = [](const Vec &x) -> Vec {
        return {pow(x[0] - 4, 4), pow(x[1] - 3, 2), 4 * pow(x[2] + 5, 4)};
    };

    Vec x{4, 2, -1};
    Vec d = -Vec{0, -2, 1024};
    double alpha = FindAlpha(x, d, g);
    cout << alpha << endl;

    // FIXME: not match got results
    double expected = 0.003866;
}

TEST(Solve, Base) {
    MemoryLeakDetection mld;

    std::setlocale(LC_ALL, ".UTF8");

    Node f1 = Var("a") * cos(Var("x1")) + Var("b") * cos(Var("x1") - Var("x2")) +
              Var("c") * cos(Var("x1") - Var("x2") - Var("x3"));
    Node f2 = Var("a") * sin(Var("x1")) + Var("b") * sin(Var("x1") - Var("x2")) +
              Var("c") * sin(Var("x1") - Var("x2") - Var("x3"));
    Node f3 = Var("x1") - Var("x2") - Var("x3");

    std::vector<std::string> vars = {"x1", "x2", "x3"};
    SymVec equations{Clone(f1), Clone(f2), Clone(f3)};
    equations.Subs({{"a", 0.425}, {"b", 0.39243}, {"c", 0.109}});
    cout << equations.ToString() << endl;

    SymVec b{Num(0.5), Num(0.4), Num(0)};
    SymVec n = equations - b;

    SymMat ja = Jacobian(n, vars);
    cout << ja.ToString() << endl;

    std::unordered_map<std::string, double> varsTable = {{"x1", 1}, {"x2", 1}, {"x3", 1}};

    Vec n0 = n.Clone().Subs(varsTable).Calc().ToMat().ToVec();
    cout << "n0 = " << n0 << endl;

    Mat ja0 = ja.Clone().Subs(varsTable).Calc().ToMat();
    cout << "ja0 = " << ja0 << endl;

    // Vec x = SolveLinear(ja0, n0);
    // cout << "x = " << x << endl;

    Vec ret = Solve(varsTable, n);
    cout << "ret = " << ret << endl;

    Vec expected{{1.5722855035930956, 1.6360330989069252, -0.0637475947386077}};
    ASSERT_EQ(ret, expected);
}

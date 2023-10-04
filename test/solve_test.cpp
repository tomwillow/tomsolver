#include "symmat.h"
#include "functions.h"
#include "linear.h"

#include "memory_leak_detection.h"

#include <gtest/gtest.h>

#include <iostream>

using namespace std;
using namespace tomsolver;

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
    auto n = equations - b;

    SymMat ja = Jacobian(n, vars);
    cout << ja.ToString() << endl;

    std::unordered_map<std::string, double> ret = {{"x1", 0}, {"x2", 0}, {"x3", 0}};

    Vec n0 = n.Clone().Subs(ret).Calc().ToMat().ToVec();
    cout << "n0 = " << n0 << endl;

    Mat ja0 = ja.Clone().Subs(ret).Calc().ToMat();
    cout << "ja0 = " << ja0 << endl;

    // Vec x = SolveLinear(ja0, n0);
    // cout << "x = " << x << endl;
}

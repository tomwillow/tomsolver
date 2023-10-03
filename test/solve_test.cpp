#include "matrix.h"
#include "functions.h"

#include "memory_leak_detection.h"

#include <gtest/gtest.h>

#include <iostream>

using namespace std;
using namespace tomsolver;

TEST(Solve, Base) {
    MemoryLeakDetection mld;

    Node f1 = Var("a") * cos(Var("x1")) + Var("b") * cos(Var("x1") - Var("x2")) +
              Var("c") * cos(Var("x1") - Var("x2") - Var("x3"));
    Node f2 = Var("a") * sin(Var("x1")) + Var("b") * sin(Var("x1") - Var("x2")) +
              Var("c") * sin(Var("x1") - Var("x2") - Var("x3"));
    Node f3 = Var("x1") - Var("x2") - Var("x3");

    std::vector<std::string> vars = {"x1", "x2", "x3"};
    Vec equations{Clone(f1), Clone(f2), Clone(f3)};
    equations.Subs({{"a", 0.425}, {"b", 0.39243}, {"c", 0.109}});
    cout << equations.ToString() << endl;

    Vec b{Num(0.5), Num(0.4), Num(0)};
    // auto n = equations - b;

    Mat ja = Jacobian(equations, vars);
    cout << ja.ToString() << endl;
}

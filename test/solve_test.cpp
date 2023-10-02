#include "tomsolver.h"

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
    std::vector<Node> equations;
    equations.push_back(Clone(f1));
    equations.push_back(Clone(f2));
    equations.push_back(Clone(f3));

    for (auto &eq : equations) {
        for (auto &var : vars) {
            cout << Diff(eq, var)->ToString() << endl;
        }
    }
}

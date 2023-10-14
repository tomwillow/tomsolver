#include "symmat.h"
#include "functions.h"

#include "memory_leak_detection.h"

#include <gtest/gtest.h>

#include <random>

using namespace tomsolver;

using std::cout;
using std::endl;

TEST(SymMat, Base) {
    MemoryLeakDetection mld;

    SymVec a{Var("a"), Var("b"), Var("c")};

    cout << a.ToString() << endl;

    Node x = Var("x");
    Node y = Var("y");
    Node f1 = (sin(x) ^ Num(2)) + x * y + y - Num(3);
    Node f2 = Num(4) * x + (y ^ Num(2));

    SymVec f{std::move(f1), std::move(f2)};

    cout << f.ToString() << endl;
}

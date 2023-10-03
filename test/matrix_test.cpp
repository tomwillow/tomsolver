#include "matrix.h"
#include "functions.h"

#include "memory_leak_detection.h"

#include <gtest/gtest.h>

#include <random>

using namespace std;
using namespace tomsolver;

TEST(Matrix, Base) {
    MemoryLeakDetection mld;

    Vec a{Var("a"), Var("b"), Var("c")};

    cout << a.ToString() << endl;

    Node x = Var("x");
    Node y = Var("y");
    Node f1 = (sin(x) ^ Num(2)) + x * y + y - Num(3);
    Node f2 = Num(4) * x + (y ^ Num(2));

    Vec f{std::move(f1), std::move(f2)};

    cout << f.ToString() << endl;
}

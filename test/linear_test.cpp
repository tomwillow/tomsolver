#include "linear.h"

#include "memory_leak_detection.h"

#include <gtest/gtest.h>

using namespace std;
using namespace tomsolver;

TEST(Linear, Base) {
    MemoryLeakDetection mld;

    TMatrix A = {{2, 1, -5, 1}, {1, -5, 0, 7}, {0, 2, 1, -1}, {1, 6, -1, -4}};
    TVector b = {13, -9, 6, 0};

    auto x = SolveLinear(A, b);

    std::copy(x.begin(), x.end(), std::ostream_iterator<double>(std::cout, ", "));
    cout << endl;

    TVector expected = {-66.5556, 25.6667, -18.7778, 26.5556};
}
#pragma once

#include <vector>

namespace tomsolver {

using TMatrix = std::vector<std::vector<double>>;
using TVector = std::vector<double>;

TVector SolveLinear(TMatrix &A, TVector &b);

} // namespace tomsolver

#include "helper.h"

#include "node.h"
#include "functions.h"

#include <cmath>
#include <random>
#include <chrono>

using namespace std;
using namespace tomsolver;

std::pair<Node, double> tomsolver::CreateRandomExpresionTree(int len) {
    auto seed = static_cast<unsigned int>(chrono::high_resolution_clock::now().time_since_epoch().count());
    cout << "seed = " << seed << endl;
    default_random_engine eng(seed);

    vector<MathOperator> ops{MathOperator::MATH_POSITIVE, MathOperator::MATH_NEGATIVE, MathOperator::MATH_ADD,
                             MathOperator::MATH_SUB,      MathOperator::MATH_MULTIPLY, MathOperator::MATH_DIVIDE,
                             MathOperator::MATH_SIN,      MathOperator::MATH_COS,      MathOperator::MATH_TAN,
                             MathOperator::MATH_ARCSIN,   MathOperator::MATH_ARCCOS,   MathOperator::MATH_ARCTAN};
    uniform_int_distribution<int> unifOp(0, static_cast<int>(ops.size()) - 1);
    uniform_real_distribution<double> unifNum(-100.0, 100.0);
    double v = unifNum(eng);
    auto node = Num(v);

    for (int j = 0; j < len;) {
        double num = unifNum(eng);
        auto op = ops[unifOp(eng)];

        bool frontOrBack = unifOp(eng) % 2;
        switch (op) {
        case MathOperator::MATH_POSITIVE: {
            node = +std::move(node);
            break;
        }
        case MathOperator::MATH_NEGATIVE: {
            v = -v;
            node = -std::move(node);
            break;
        }
        case MathOperator::MATH_ADD:
            if (frontOrBack) {
                v = num + v;
                node = Num(num) + std::move(node);
            } else {
                v += num;
                node += Num(num);
            }
            break;
        case MathOperator::MATH_SUB:
            if (frontOrBack) {
                v = num - v;
                node = Num(num) - std::move(node);
            } else {
                v -= num;
                node -= Num(num);
            }
            break;
        case MathOperator::MATH_MULTIPLY:
            if (frontOrBack) {
                v = num * v;
                node = Num(num) * std::move(node);
            } else {
                v *= num;
                node *= Num(num);
            }
            break;
        case MathOperator::MATH_DIVIDE:
            if (frontOrBack) {
                if (v == 0) {
                    continue;
                }
                v = num / v;
                node = Num(num) / std::move(node);
            } else {
                if (num == 0) {
                    continue;
                }
                v /= num;
                node /= Num(num);
            }
            break;
        case MathOperator::MATH_SIN:
            v = std::sin(v);
            node = sin(std::move(node));
            break;
        case MathOperator::MATH_COS:
            v = std::cos(v);
            node = cos(std::move(node));
            break;
        case MathOperator::MATH_TAN:
            v = std::tan(v);
            node = tan(std::move(node));
            break;
        case MathOperator::MATH_ARCSIN:
            if (v < -1.0 || v > 1.0) {
                continue;
            }
            v = std::asin(v);
            node = asin(std::move(node));
            break;
        case MathOperator::MATH_ARCCOS:
            if (v < -1.0 || v > 1.0) {
                continue;
            }
            v = std::acos(v);
            node = acos(std::move(node));
            break;
        case MathOperator::MATH_ARCTAN:
            v = std::atan(v);
            node = atan(std::move(node));
            break;
        default:
            assert(0);
        }

        ++j;
    }
    return {std::move(node), v};
}

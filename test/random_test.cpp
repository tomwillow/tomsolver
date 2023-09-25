#include "tomsolver.h"

#include "memory_leak_detection.h"

#include <gtest/gtest.h>

#include <random>

using namespace std;
using namespace tomsolver;

std::pair<Node, double> CreateRandomExpresionTree(int len) {
    auto seed = static_cast<unsigned int>(chrono::high_resolution_clock::now().time_since_epoch().count());
    cout << "seed = " << seed << endl;
    default_random_engine eng(seed);

    vector<MathOperator> ops{MathOperator::MATH_ADD,    MathOperator::MATH_SUB,    MathOperator::MATH_MULTIPLY,
                             MathOperator::MATH_DIVIDE, MathOperator::MATH_SIN,    MathOperator::MATH_COS,
                             MathOperator::MATH_TAN,    MathOperator::MATH_ARCSIN, MathOperator::MATH_ARCCOS,
                             MathOperator::MATH_ARCTAN};
    uniform_int_distribution<int> unifOp(0, static_cast<int>(ops.size()) - 1);
    uniform_real_distribution<double> unifNum(-100.0, 100.0);
    double v = unifNum(eng);
    auto node = Num(v);

    for (int j = 0; j < len;) {
        double num = unifNum(eng);
        auto op = ops[unifOp(eng)];

        bool frontOrBack = unifOp(eng) % 2;
        switch (op) {
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
            v = sin(v);
            node = std::move(sin(std::move(node)));
            break;
        case MathOperator::MATH_COS:
            v = cos(v);
            node = std::move(cos(std::move(node)));
            break;
        case MathOperator::MATH_TAN:
            v = tan(v);
            node = std::move(tan(std::move(node)));
            break;
        case MathOperator::MATH_ARCSIN:
            if (v < -1.0 || v > 1.0) {
                continue;
            }
            v = asin(v);
            node = std::move(asin(std::move(node)));
            break;
        case MathOperator::MATH_ARCCOS:
            if (v < -1.0 || v > 1.0) {
                continue;
            }
            v = acos(v);
            node = std::move(acos(std::move(node)));
            break;
        case MathOperator::MATH_ARCTAN:
            v = atan(v);
            node = std::move(atan(std::move(node)));
            break;
        default:
            assert(0);
        }

        ++j;
    }
    return {std::move(node), v};
}

TEST(Node, Random) {
    MemoryLeakDetection mld;

    int maxCount = 10;

    auto seed = static_cast<unsigned int>(chrono::high_resolution_clock::now().time_since_epoch().count());
    cout << "seed = " << seed << endl;
    default_random_engine eng(seed);

    uniform_int_distribution<int> unifCount(1, maxCount);
    for (int i = 0; i < 10; ++i) {
        int count = unifCount(eng);

        auto pr = CreateRandomExpresionTree(count);
        Node &node = pr.first;
        double v = pr.second;

        node->CheckParent();

        double result = node->Vpa();
        cout << node->ToString() << endl;
        cout << "\t result = " << result << endl;
        cout << "\t expected = " << v << endl;
        ASSERT_DOUBLE_EQ(result, v);

        // clone
        Node n2 = Clone(node);
        ASSERT_DOUBLE_EQ(result, n2->Vpa());
        n2->CheckParent();

        cout << endl;
    }
}

TEST(Node, DoNotStackOverFlow) {
    MemoryLeakDetection mld;

    std::setlocale(LC_ALL, ".UTF8");

    // 关掉定义域检查
    GetConfig().checkDomain = false;

    // 构造一个随机的长表达式
    auto pr = CreateRandomExpresionTree(10000);
    Node &node = pr.first;
    double v = pr.second;

    // clone，不应爆栈
    Node n2 = Clone(node);

    double result = node->Vpa();

    // 输出表达式字符串，不应爆栈
    string s = node->ToString();

    cout << "\t result = " << result << endl;
    cout << "\t expected = " << v << endl;
    ASSERT_DOUBLE_EQ(result, v);

    GetConfig().checkDomain = true;
}

TEST(Vec, Base) {
    vector<Node> vec;
}
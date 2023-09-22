#include "tomsolver.h"

#include "memory_leak_detection.h"

#include <gtest/gtest.h>

#include <random>

using namespace std;
using namespace tomsolver;

TEST(Node, Num) {
    MemoryLeakDetection mld;

    auto n = Num(10);
    cout << n->ToString() << endl;
    ASSERT_EQ(n->ToString(), "10.000000");

    // 右值+右值
    auto n2 = Num(1) + Num(2);
    cout << n2->ToString() << endl;
    ASSERT_EQ(n2->ToString(), "1.000000+2.000000");

    // 左值+左值
    auto n3 = n + n2;
    cout << n3->ToString() << endl;
    ASSERT_EQ(n3->ToString(), "10.000000+1.000000+2.000000");
    cout << n3->ToString() << endl;
    ASSERT_EQ(n3->ToString(), "10.000000+1.000000+2.000000");

    // 前面的 n n2 不应被释放
    ASSERT_EQ(n->ToString(), "10.000000");
    ASSERT_EQ(n2->ToString(), "1.000000+2.000000");

    // 左值+右值
    auto n4 = n + Num(3);
    ASSERT_EQ(n4->ToString(), "10.000000+3.000000");
    // 前面的 n 不应被释放
    ASSERT_EQ(n->ToString(), "10.000000");

    // 右值+左值
    auto n5 = Num(3) + n;
    ASSERT_EQ(n5->ToString(), "3.000000+10.000000");
    // 前面的 n 不应被释放
    ASSERT_EQ(n->ToString(), "10.000000");
}

TEST(Node, Var) {
    MemoryLeakDetection mld;

    ASSERT_ANY_THROW(Var("0a"));

    Var("a");
    Var("a0");
    Var("_");
    Var("_a");
    Var("_1");

    auto expr = Var("a") - Num(1);
    cout << expr << endl;
    ASSERT_EQ(expr->ToString(), "a-1.000000");
}

TEST(Node, AddEqual) {
    MemoryLeakDetection mld;

    auto n = Num(10);

    n += Num(1);
    ASSERT_EQ(n->ToString(), "10.000000+1.000000");

    auto n2 = Num(20);
    n += n2;
    ASSERT_EQ(n->ToString(), "10.000000+1.000000+20.000000");

    // 前面的 n2 不应被释放
    ASSERT_EQ(n2->ToString(), "20.000000");
}

TEST(Node, SubEqual) {
    MemoryLeakDetection mld;

    auto n = Num(10);

    n -= Num(1);
    ASSERT_EQ(n->ToString(), "10.000000-1.000000");

    auto n2 = Num(20);
    n -= n2;
    ASSERT_EQ(n->ToString(), "10.000000-1.000000-20.000000");

    // 前面的 n2 不应被释放
    ASSERT_EQ(n2->ToString(), "20.000000");
}

TEST(Node, MulEqual) {
    MemoryLeakDetection mld;

    auto n = Num(10);

    n *= Num(1);
    ASSERT_EQ(n->ToString(), "10.000000*1.000000");

    auto n2 = Num(20);
    n *= n2;
    ASSERT_EQ(n->ToString(), "10.000000*1.000000*20.000000");

    // 前面的 n2 不应被释放
    ASSERT_EQ(n2->ToString(), "20.000000");
}

TEST(Node, DivEqual) {
    MemoryLeakDetection mld;

    auto n = Num(10);

    n /= Num(1);
    ASSERT_EQ(n->ToString(), "10.000000/1.000000");

    auto n2 = Num(20);
    n /= n2;
    ASSERT_EQ(n->ToString(), "10.000000/1.000000/20.000000");

    // 前面的 n2 不应被释放
    ASSERT_EQ(n2->ToString(), "20.000000");
}

TEST(Node, Multiply) {
    MemoryLeakDetection mld;

    {
        auto expr = Var("a") + Var("b") * Var("c");
        cout << expr << endl;
        ASSERT_EQ(expr->ToString(), "a+b*c");
    }

    {
        auto expr = Num(1) + Num(2) * Num(3);
        cout << expr << " = " << expr->Vpa() << endl;
        ASSERT_DOUBLE_EQ(expr->Vpa(), 7.0);
    }

    {
        auto expr = (Num(1) + Num(2)) * Num(3);
        cout << expr << " = " << expr->Vpa() << endl;
        ASSERT_DOUBLE_EQ(expr->Vpa(), 9.0);
    }
}

TEST(Node, Divide) {
    MemoryLeakDetection mld;

    {
        auto expr = Var("a") + Var("b") / Var("c");
        cout << expr << endl;
        ASSERT_EQ(expr->ToString(), "a+b/c");
    }

    {
        auto expr = Num(1) + Num(2) / Num(4);
        cout << expr << " = " << expr->Vpa() << endl;
        ASSERT_DOUBLE_EQ(expr->Vpa(), 1.5);
    }

    {
        auto expr = (Num(1) + Num(2)) / Num(4);
        cout << expr << " = " << expr->Vpa() << endl;
        ASSERT_DOUBLE_EQ(expr->Vpa(), 0.75);
    }

    auto expr = Num(1) / Num(0);
    ASSERT_THROW(expr->Vpa(), MathError);
}

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

        double result = node->Vpa();
        cout << node->ToString() << endl;
        cout << "\t result = " << result << endl;
        cout << "\t expected = " << v << endl;
        ASSERT_DOUBLE_EQ(result, v);
        cout << endl;
    }
}

TEST(Node, DoNotStackOverFlow) {
    MemoryLeakDetection mld;

    GetConfig().checkDomain = false;

    auto pr = CreateRandomExpresionTree(10000);
    Node &node = pr.first;
    double v = pr.second;

    std::setlocale(LC_ALL, ".UTF8");
    double result = node->Vpa();
    // cout << node->ToString() << endl;
    cout << "\t result = " << result << endl;
    cout << "\t expected = " << v << endl;
    ASSERT_DOUBLE_EQ(result, v);

    GetConfig().checkDomain = true;
}

TEST(Vec, Base) {
    vector<Node> vec;
}
#include "tomsolver/tomsolver.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <random>
#include <chrono>
#include <deque>

using namespace tomsolver;

using std::cout;
using std::deque;
using std::endl;

#ifdef WIN32
#else
class MemoryLeakDetection final {
public:
    MemoryLeakDetection() {}
};
#endif

#ifdef WIN32

#include <windows.h>

#undef max
#undef min

#define _CRTDBG_MAP_ALLOC // to get more details
#include <stdlib.h>
#include <crtdbg.h> //for malloc and free

#include <gtest/gtest.h>

#include <iostream>
#include <cassert>

class MemoryLeakDetection final {
public:
    MemoryLeakDetection() {
        _CrtMemCheckpoint(&sOld); // take a snapshot
    }

    ~MemoryLeakDetection() {
        _CrtMemCheckpoint(&sNew);                    // take a snapshot
        if (_CrtMemDifference(&sDiff, &sOld, &sNew)) // if there is a difference
        {
            // OutputDebugString(TEXT("-----------_CrtMemDumpStatistics ---------"));
            //_CrtMemDumpStatistics(&sDiff);
            // OutputDebugString(TEXT("-----------_CrtMemDumpAllObjectsSince ---------"));
            //_CrtMemDumpAllObjectsSince(&sOld);
            // OutputDebugString(TEXT("-----------_CrtDumpMemoryLeaks ---------"));
            _CrtDumpMemoryLeaks();

            EXPECT_TRUE(0 && "Memory leak is detected! See debug output for detail.");
        }
    }

    void SetBreakAlloc(long index) const noexcept {
        (index);
        _CrtSetBreakAlloc(index);
    }

private:
    _CrtMemState sOld;
    _CrtMemState sNew;
    _CrtMemState sDiff;
};

#endif

#include <cmath>
#include <random>
#include <chrono>

using std::cout;
using std::endl;

namespace tomsolver {

std::pair<Node, double> CreateRandomExpresionTree(int len) {
    auto seed = static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    cout << "seed = " << seed << endl;
    std::default_random_engine eng(seed);

    std::vector<MathOperator> ops{MathOperator::MATH_POSITIVE, MathOperator::MATH_NEGATIVE, MathOperator::MATH_ADD,
                                  MathOperator::MATH_SUB,      MathOperator::MATH_MULTIPLY, MathOperator::MATH_DIVIDE,
                                  MathOperator::MATH_SIN,      MathOperator::MATH_COS,      MathOperator::MATH_TAN,
                                  MathOperator::MATH_ARCSIN,   MathOperator::MATH_ARCCOS,   MathOperator::MATH_ARCTAN};
    std::uniform_int_distribution<int> unifOp(0, static_cast<int>(ops.size()) - 1);
    std::uniform_real_distribution<double> unifNum(-100.0, 100.0);
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

} // namespace tomsolver
TEST(Diff, Base) {
    MemoryLeakDetection mld;

    Node n = Var("a");
    ASSERT_TRUE(Diff(n, "a")->Equal(Num(1)));
    ASSERT_TRUE(Diff(n, "b")->Equal(Num(0)));

    ASSERT_TRUE(Diff(Num(1), "a")->Equal(Num(0)));

    // diff(a+b, a) == 1
    Node n2 = n + Var("b");
    ASSERT_TRUE(Diff(n2, "a")->Equal(Num(1)));
}
TEST(Diff, Negative) {
    MemoryLeakDetection mld;

    Node n = -Var("a");
    auto dn = Diff(n, "a");
    ASSERT_TRUE(dn->Equal(Num(-1)));
    ASSERT_TRUE(Diff(n, "b")->Equal(Num(0)));

    ASSERT_TRUE(Diff(-Num(1), "a")->Equal(Num(0)));

    // diff(-a+ -b, a) == -1
    Node n2 = n + -Var("b");
    auto dn2a = Diff(n2, "a");
    ASSERT_TRUE(dn2a->Equal(Num(-1)));
    auto dn2b = Diff(n2, "b");
    ASSERT_TRUE(dn2b->Equal(Num(-1)));

    // diff(-a+ +b, a) == -1
    Node n3 = n + +Var("b");
    ASSERT_TRUE(Diff(n3, "a")->Equal(Num(-1)));
    ASSERT_TRUE(Diff(n3, "b")->Equal(Num(1)));
}
TEST(Diff, Sin) {
    MemoryLeakDetection mld;

    {
        // sin'x = cos x
        Node n = sin(Var("x"));
        Node dn = Diff(n, "x");
        dn->CheckParent();
        cout << dn->ToString() << endl;
        ASSERT_TRUE(dn->Equal(cos(Var("x"))));
    }
}
TEST(Diff, Cos) {
    MemoryLeakDetection mld;

    {
        // cos'x = -sin x
        Node n = cos(Var("x"));
        Node dn = Diff(n, "x");
        dn->CheckParent();
        cout << dn->ToString() << endl;
        ASSERT_TRUE(dn->Equal(-sin(Var("x"))));
    }
}
TEST(Diff, Exp) {
    MemoryLeakDetection mld;

    {
        // (e^x)' = e^x
        Node n = exp(Var("x"));
        Node dn = Diff(n, "x");
        dn->CheckParent();
        cout << dn->ToString() << endl;
        ASSERT_TRUE(dn->Equal(exp(Var("x"))));
    }

    {
        // (e^sin(x))' = e^sin(x)*cos(x)
        Node n = exp(sin(Var("x")));
        Node dn = Diff(n, "x");
        dn->CheckParent();
        cout << dn->ToString() << endl;
        ASSERT_TRUE(dn->Equal(exp(sin(Var("x"))) * cos(Var("x"))));
    }
}
TEST(Diff, Multiply) {
    MemoryLeakDetection mld;

    // diff(5*a, a) == 5
    ASSERT_TRUE(Diff(Num(5) * Var("a"), "a")->Equal(Num(5)));

    // diff(b*5, b) == 5
    ASSERT_TRUE(Diff(Var("b") * Num(5), "b")->Equal(Num(5)));

    {
        // diff(a*b, a) == b
        Node n = Var("a") * Var("b");
        Node dn = Diff(n, "a");
        dn->CheckParent();
        cout << dn->ToString() << endl;
        ASSERT_TRUE(dn->Equal(Var("b")));
    }

    {
        // diff(a*b*a, a) ==
        Node n = Var("a") * Var("b") * Var("a");
        Node dn = Diff(n, "a");
        dn->CheckParent();
        cout << dn->ToString() << endl;
    }
}
TEST(Diff, Log) {
    MemoryLeakDetection mld;

    {
        // log(x)' = 1/x
        Node n = log(Var("x"));
        Node dn = Diff(n, "x");
        dn->CheckParent();
        cout << dn->ToString() << endl;
        ASSERT_TRUE(dn->Equal(Num(1) / Var("x")));
    }

    {
        // log(sin(x))' = 1/x * cos(x)
        Node n = log(sin(Var("x")));
        Node dn = Diff(n, "x");
        dn->CheckParent();
        cout << dn->ToString() << endl;
        ASSERT_TRUE(dn->Equal(Num(1) / sin(Var("x")) * cos(Var("x"))));
    }
}
TEST(Diff, LogChain) {
    MemoryLeakDetection mld;
    {
        // (x*ln(x))' = ln(x)+1
        Node n = Var("x") * log(Var("x"));
        Node dn = Diff(n, "x");
        dn->CheckParent();
        cout << dn->ToString() << endl;
        ASSERT_EQ(dn->ToString(), "log(x)+x*1/x"); // FIXME: 化简
    }
}
TEST(Diff, Power) {
    MemoryLeakDetection mld;

    {
        // (x^a)' = a*x^(a-1)
        Node n = Var("x") ^ Num(5);
        Node dn = Diff(n, "x");
        dn->CheckParent();
        cout << dn->ToString() << endl;
        ASSERT_TRUE(dn->Equal(Num(5) * (Var("x") ^ Num(4))));
    }

    {
        // (a^x)' = a^x * ln(a)  when a>0 and a!=1
        Node n = Num(3) ^ Var("x");
        Node dn = Diff(n, "x");
        dn->CheckParent();
        cout << dn->ToString() << endl;
        Node expect = (Num(3) ^ Var("x")) * Num(std::log(3));
        ASSERT_TRUE(dn->Equal(expect));
    }

    {
        // (u^v)' = ( e^(v*ln(u)) )' = e^(v*ln(u)) * (v*ln(u))'

        Node n = Var("x") ^ Var("x");
        Node dn = Diff(n, "x");
        dn->CheckParent();
        cout << dn->ToString() << endl;
        ASSERT_EQ(dn->ToString(), "x^x*(log(x)+x*1/x)"); // FIXME: 化简
    }

    {
        // (u^v)' = ( e^(v*ln(u)) )' = e^(v*ln(u)) * (v*ln(u))'

        Node n = sin(Var("x")) ^ cos(Var("x"));
        Node dn = Diff(n, "x");
        dn->CheckParent();
        cout << dn->ToString() << endl;
        ASSERT_EQ(dn->ToString(), "sin(x)^cos(x)*(-(sin(x))*log(sin(x))+cos(x)*1/sin(x)*cos(x))"); // FIXME: 化简
    }
}
TEST(Diff, Combine) {
    MemoryLeakDetection mld;

    {
        // diff(sin(a*b+c)*1*a, a)
        Node n = sin(Var("a") * Var("b") + Var("c")) * Num(1) * Var("a");
        Node dn = Diff(n, "a");
        dn->CheckParent();
        cout << dn->ToString() << endl;
        ASSERT_EQ(dn->ToString(), "cos(a*b+c)*b*a+sin(a*b+c)");
    }

    {
        // diff(sin(cos(x)+sin(x)), x) =
        Node n = sin(cos(Var("x")) + sin(Var("x")));
        Node dn = Diff(n, "x");
        dn->CheckParent();
        cout << dn->ToString() << endl;
        ASSERT_EQ(dn->ToString(), "cos(cos(x)+sin(x))*(-(sin(x))+cos(x))");
    }
}

TEST(Function, Trigonometric) {
    MemoryLeakDetection mld;

    int count = 100;

    auto seed = static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    cout << "seed = " << seed << endl;
    std::default_random_engine eng(seed);
    std::uniform_real_distribution<double> unifNum;

    for (int i = 0; i < count; ++i) {
        double num = unifNum(eng);
        ASSERT_DOUBLE_EQ(sin(Num(num))->Vpa(), sin(num));
        ASSERT_DOUBLE_EQ(cos(Num(num))->Vpa(), cos(num));
        ASSERT_DOUBLE_EQ(tan(Num(num))->Vpa(), tan(num));
        ASSERT_DOUBLE_EQ(asin(Num(num))->Vpa(), asin(num));
        ASSERT_DOUBLE_EQ(acos(Num(num))->Vpa(), acos(num));
        ASSERT_DOUBLE_EQ(atan(Num(num))->Vpa(), atan(num));
        ASSERT_DOUBLE_EQ(sqrt(Num(num))->Vpa(), sqrt(num));
        ASSERT_DOUBLE_EQ(log(Num(num))->Vpa(), log(num));
        ASSERT_DOUBLE_EQ(log2(Num(num))->Vpa(), log2(num));
        ASSERT_DOUBLE_EQ(log10(Num(num))->Vpa(), log10(num));
        ASSERT_DOUBLE_EQ(exp(Num(num))->Vpa(), exp(num));
    }
}
TEST(Function, InvalidNumber) {
    MemoryLeakDetection mld;
    double inf = std::numeric_limits<double>::infinity();
    double inf2 = -std::numeric_limits<double>::infinity();
    double nan = std::numeric_limits<double>::quiet_NaN();
    double dblMax = std::numeric_limits<double>::max();

#define MY_ASSERT_THROW(statement, shouldThrow)                                                                        \
    if (shouldThrow) {                                                                                                 \
        try {                                                                                                          \
            statement;                                                                                                 \
            FAIL();                                                                                                    \
        } catch (const MathError &err) {                                                                               \
            std::cerr << "[Expected Exception]" << err.what() << std::endl;                                            \
        }                                                                                                              \
    } else {                                                                                                           \
        try {                                                                                                          \
            statement;                                                                                                 \
        } catch (const MathError &err) {                                                                               \
            std::cerr << "[Unexpected Exception]" << err.what() << std::endl;                                          \
            FAIL();                                                                                                    \
        }                                                                                                              \
    }

    auto Test = [&](bool shouldThrow) {
        MY_ASSERT_THROW((Num(inf) + Num(1))->Vpa(), shouldThrow);
        MY_ASSERT_THROW((Num(inf2) + Num(1))->Vpa(), shouldThrow);
        MY_ASSERT_THROW((Num(nan) + Num(1))->Vpa(), shouldThrow);

        MY_ASSERT_THROW((Num(1) / Num(0))->Vpa(), shouldThrow);

        // pow(DBL_DOUELB, 2)
        MY_ASSERT_THROW((Num(dblMax) ^ Num(2))->Vpa(), shouldThrow);

        MY_ASSERT_THROW(asin(Num(1.1))->Vpa(), shouldThrow);
        MY_ASSERT_THROW(asin(Num(-1.1))->Vpa(), shouldThrow);

        MY_ASSERT_THROW(acos(Num(1.1))->Vpa(), shouldThrow);
        MY_ASSERT_THROW(acos(Num(-1.1))->Vpa(), shouldThrow);

        MY_ASSERT_THROW(sqrt(Num(-0.1))->Vpa(), shouldThrow);

        MY_ASSERT_THROW(log(Num(0))->Vpa(), shouldThrow);
        MY_ASSERT_THROW(log2(Num(0))->Vpa(), shouldThrow);
        MY_ASSERT_THROW(log10(Num(0))->Vpa(), shouldThrow);
    };

    // 默认配置下，无效值应该抛异常
    Test(true);

    // 手动关闭无效值检查，不应抛异常
    {
        GetConfig().throwOnInvalidValue = false;

        Test(false);

        // 恢复配置
        GetConfig().Reset();
    }
}
TEST(Function, ToString) {
    MemoryLeakDetection mld;

    Node f = Var("r") * sin(Var("omega") / Num(2.0) + Var("phi")) + Var("c");

    ASSERT_EQ(f->ToString(), "r*sin(omega/2+phi)+c");
}

TEST(Linear, Base) {
    MemoryLeakDetection mld;

    Mat A = {{2, 1, -5, 1}, {1, -5, 0, 7}, {0, 2, 1, -1}, {1, 6, -1, -4}};
    Vec b = {13, -9, 6, 0};

    auto x = SolveLinear(A, b);

    Vec expected = {-66.5555555555555429, 25.6666666666666643, -18.777777777777775, 26.55555555555555};

    ASSERT_EQ(x, expected);
}

TEST(Mat, Inverse) {
    MemoryLeakDetection mld;

    {
        Mat A = {{1, 2}, {3, 4}};
        auto inv = A.Inverse();
        Mat expected = {{-2, 1}, {1.5, -0.5}};
        ASSERT_EQ(inv, expected);
    }
    {
        Mat A = {{1, 2, 3}, {4, 5, 6}, {-2, 7, 8}};
        auto inv = A.Inverse();
        Mat expected = {{-0.083333333333333, 0.208333333333333, -0.125000000000000},
                        {-1.833333333333333, 0.583333333333333, 0.250000000000000},
                        {1.583333333333333, -0.458333333333333, -0.125000000000000}};
        ASSERT_EQ(inv, expected);
    }
    {
        Mat A = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
        try {
            auto inv = A.Inverse();
            FAIL();
        } catch (const MathError &e) {
            cout << "[Expected]" << e.what() << endl;
        }
    }
}
TEST(Mat, PositiveDetermine) {
    MemoryLeakDetection mld;

    {
        Mat A = {{1, 1, 1, 1}, {1, 2, 3, 4}, {1, 3, 6, 10}, {1, 4, 10, 20}};
        ASSERT_TRUE(A.PositiveDetermine());
    }
    {
        Mat A = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
        ASSERT_TRUE(!A.PositiveDetermine());
    }
}

TEST(Node, Num) {
    MemoryLeakDetection mld;

    auto n = Num(10);
    cout << n->ToString() << endl;
    ASSERT_EQ(n->ToString(), "10");

    // 右值+右值
    auto n2 = Num(1) + Num(2);
    cout << n2->ToString() << endl;
    ASSERT_EQ(n2->ToString(), "1+2");

    // 左值+左值
    auto n3 = n + n2;
    n3->CheckParent();
    cout << n3->ToString() << endl;
    ASSERT_EQ(n3->ToString(), "10+1+2");
    cout << n3->ToString() << endl;
    ASSERT_EQ(n3->ToString(), "10+1+2");

    // 前面的 n n2 不应被释放
    ASSERT_EQ(n->ToString(), "10");
    ASSERT_EQ(n2->ToString(), "1+2");

    // 左值+右值
    auto n4 = n + Num(3);
    ASSERT_EQ(n4->ToString(), "10+3");
    // 前面的 n 不应被释放
    ASSERT_EQ(n->ToString(), "10");

    // 右值+左值
    auto n5 = Num(3) + n;
    ASSERT_EQ(n5->ToString(), "3+10");
    // 前面的 n 不应被释放
    ASSERT_EQ(n->ToString(), "10");

    n->CheckParent();
    n2->CheckParent();
    n4->CheckParent();
    n5->CheckParent();
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
    ASSERT_EQ(expr->ToString(), "a-1");

    expr->CheckParent();
}
TEST(Node, Clone) {
    MemoryLeakDetection mld;

    Node n = Var("a") + Var("b") * Var("c");
    n->CheckParent();

    Node n2 = Clone(n);
    n2->CheckParent();

    ASSERT_EQ(n->ToString(), "a+b*c");
    ASSERT_EQ(n2->ToString(), "a+b*c");
}
TEST(Node, Move) {
    MemoryLeakDetection mld;

    Node n = Var("a") + Var("b") * Var("c");
    Node n2 = Move(n);

    ASSERT_EQ(n, nullptr);
    ASSERT_EQ(n2->ToString(), "a+b*c");

    n2->CheckParent();
}
TEST(Node, AddEqual) {
    MemoryLeakDetection mld;

    auto n = Num(10);

    n += Num(1);
    ASSERT_EQ(n->ToString(), "10+1");

    auto n2 = Num(20);
    n += n2;
    ASSERT_EQ(n->ToString(), "10+1+20");

    // 前面的 n2 不应被释放
    ASSERT_EQ(n2->ToString(), "20");

    n->CheckParent();
    n2->CheckParent();
}
TEST(Node, Sub) {
    MemoryLeakDetection mld;

    Node n = Num(10) - Num(-10);
    ASSERT_EQ(n->ToString(), "10-(-10)");
}
TEST(Node, SubEqual) {
    MemoryLeakDetection mld;

    auto n = Num(10);

    n -= Num(1);
    ASSERT_EQ(n->ToString(), "10-1");

    auto n2 = Num(20);
    n -= n2;
    ASSERT_EQ(n->ToString(), "10-1-20");

    // 前面的 n2 不应被释放
    ASSERT_EQ(n2->ToString(), "20");

    n->CheckParent();
    n2->CheckParent();
}
TEST(Node, Negative) {
    MemoryLeakDetection mld;

    {
        Node n = -Num(10);
        ASSERT_EQ(n->ToString(), "-10");
    }

    {
        Node n = -Var("x");
        ASSERT_EQ(n->ToString(), "-x");
    }

    {
        Node n = +Var("y");
        ASSERT_EQ(n->ToString(), "+y");
    }

    {
        Node n = -(Var("x") + Num(2));
        ASSERT_EQ(n->ToString(), "-(x+2)");
    }

    {
        Node n = Var("y") + -(Var("x") + Num(2));
        ASSERT_EQ(n->ToString(), "y+-(x+2)");
    }

    {
        Node n = Var("y") + +(Var("x") + Num(2));
        ASSERT_EQ(n->ToString(), "y++(x+2)");
    }

    {
        Node n = atan(cos(-(+(-Num(87.9117553746407054) / Num(90.5933224572584663)))));
        ASSERT_DOUBLE_EQ(n->Vpa(), 0.51426347804323491);
    }
}
TEST(Node, MulEqual) {
    MemoryLeakDetection mld;

    auto n = Num(10);

    n *= Num(1);
    ASSERT_EQ(n->ToString(), "10*1");

    auto n2 = Num(20);
    n *= n2;
    ASSERT_EQ(n->ToString(), "10*1*20");

    // 前面的 n2 不应被释放
    ASSERT_EQ(n2->ToString(), "20");

    n->CheckParent();
    n2->CheckParent();
}
TEST(Node, DivEqual) {
    MemoryLeakDetection mld;

    auto n = Num(10);

    n /= Num(1);
    ASSERT_EQ(n->ToString(), "10/1");

    auto n2 = Num(20);
    n /= n2;
    ASSERT_EQ(n->ToString(), "10/1/20");

    // 前面的 n2 不应被释放
    ASSERT_EQ(n2->ToString(), "20");

    n->CheckParent();
    n2->CheckParent();
}
TEST(Node, Multiply) {
    MemoryLeakDetection mld;

    {
        auto expr = Var("a") + Var("b") * Var("c");
        cout << expr << endl;
        ASSERT_EQ(expr->ToString(), "a+b*c");

        expr->CheckParent();
    }

    {
        auto expr = (Var("a") + Var("b")) * Var("c");
        cout << expr << endl;
        ASSERT_EQ(expr->ToString(), "(a+b)*c");

        expr->CheckParent();
    }

    {
        auto expr = Num(1) + Num(2) * Num(3);
        cout << expr << " = " << expr->Vpa() << endl;
        ASSERT_DOUBLE_EQ(expr->Vpa(), 7.0);

        expr->CheckParent();
    }

    {
        auto expr = (Num(1) + Num(2)) * Num(3);
        cout << expr << " = " << expr->Vpa() << endl;
        ASSERT_DOUBLE_EQ(expr->Vpa(), 9.0);

        expr->CheckParent();
    }
}
TEST(Node, Divide) {
    MemoryLeakDetection mld;

    {
        auto expr = Var("a") + Var("b") / Var("c");
        cout << expr << endl;
        ASSERT_EQ(expr->ToString(), "a+b/c");

        expr->CheckParent();
    }

    {
        auto expr = Num(1) + Num(2) / Num(4);
        cout << expr << " = " << expr->Vpa() << endl;
        ASSERT_DOUBLE_EQ(expr->Vpa(), 1.5);

        expr->CheckParent();
    }

    {
        auto expr = (Num(1) + Num(2)) / Num(4);
        cout << expr << " = " << expr->Vpa() << endl;
        ASSERT_DOUBLE_EQ(expr->Vpa(), 0.75);

        expr->CheckParent();
    }

    auto expr = Num(1) / Num(0);
    ASSERT_THROW(expr->Vpa(), MathError);

    expr->CheckParent();
}
TEST(Node, Equal) {
    MemoryLeakDetection mld;

    Node n = Var("a") + Var("b") * Var("c");
    Node n2 = Clone(n);

    ASSERT_TRUE(n->Equal(n));
    ASSERT_TRUE(n->Equal(n2));
    ASSERT_TRUE(n2->Equal(n));

    ASSERT_TRUE(n->Equal(Var("a") + Var("b") * Var("c")));
    ASSERT_TRUE((Var("a") + Var("b") * Var("c"))->Equal(n));
}

TEST(Parse, Base) {
    MemoryLeakDetection mld;
    std::setlocale(LC_ALL, ".UTF8");

    {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("1+2");
        ASSERT_TRUE(tokens[0].node->Equal(Num(1)));
        ASSERT_TRUE(tokens[1].node->Equal(internal::Operator(MathOperator::MATH_ADD)));
        ASSERT_TRUE(tokens[2].node->Equal(Num(2)));
    }
}
TEST(Parse, Number) {
    MemoryLeakDetection mld;

    {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens(".12345");
        ASSERT_TRUE(tokens[0].node->Equal(Num(.12345)));
    }

    {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("7891.123");
        ASSERT_TRUE(tokens[0].node->Equal(Num(7891.123)));
    }

    {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("1e0");
        ASSERT_TRUE(tokens[0].node->Equal(Num(1e0)));
    }

    std::default_random_engine eng(
        static_cast<long>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));

    std::uniform_real_distribution<double> unif;

    for (int i = 0; i < 100; ++i) {
        double d = unif(eng);
        std::string expected = tomsolver::ToString(d);
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens(expected);
        ASSERT_EQ(expected, tokens[0].node->ToString());
    }
}
TEST(Parse, IllegalChar) {
    MemoryLeakDetection mld;

    try {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("1#+2");
        FAIL();
    } catch (const SingleParseError &err) {
        cout << err.what() << endl;
        ASSERT_EQ(err.GetPos(), 0);
    }

    try {
        deque<internal::Token> tokens =
            internal::ParseFunctions::ParseToTokens("a*cos(x1) + b*cos(x1-x2) + c*cos(?x1-x2-x3)");
        FAIL();
    } catch (const SingleParseError &err) {
        cout << err.what() << endl;
        ASSERT_EQ(err.GetPos(), 33);
    }
}
TEST(Parse, PositiveNegative) {
    MemoryLeakDetection mld;

    {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("1/+2");
        ASSERT_TRUE(tokens[0].node->Equal(Num(1)));
        ASSERT_TRUE(tokens[1].node->Equal(internal::Operator(MathOperator::MATH_DIVIDE)));
        ASSERT_TRUE(tokens[2].node->Equal(internal::Operator(MathOperator::MATH_POSITIVE)));
        ASSERT_TRUE(tokens[3].node->Equal(Num(2)));
    }

    {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("1/-2");
        ASSERT_TRUE(tokens[0].node->Equal(Num(1)));
        ASSERT_TRUE(tokens[1].node->Equal(internal::Operator(MathOperator::MATH_DIVIDE)));
        ASSERT_TRUE(tokens[2].node->Equal(internal::Operator(MathOperator::MATH_NEGATIVE)));
        ASSERT_TRUE(tokens[3].node->Equal(Num(2)));
    }

    {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("-1--2");
        ASSERT_TRUE(tokens[0].node->Equal(internal::Operator(MathOperator::MATH_NEGATIVE)));
        ASSERT_TRUE(tokens[1].node->Equal(Num(1)));
        ASSERT_TRUE(tokens[2].node->Equal(internal::Operator(MathOperator::MATH_SUB)));
        ASSERT_TRUE(tokens[3].node->Equal(internal::Operator(MathOperator::MATH_NEGATIVE)));
        ASSERT_TRUE(tokens[4].node->Equal(Num(2)));
    }
}
TEST(Parse, PostOrder) {
    MemoryLeakDetection mld;

    {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("1*(2-3)");

        auto postOrder = internal::ParseFunctions::InOrderToPostOrder(tokens);

        ASSERT_TRUE(postOrder[0].node->Equal(Num(1)));
        ASSERT_TRUE(postOrder[1].node->Equal(Num(2)));
        ASSERT_TRUE(postOrder[2].node->Equal(Num(3)));
        ASSERT_TRUE(postOrder[3].node->Equal(internal::Operator(MathOperator::MATH_SUB)));
        ASSERT_TRUE(postOrder[4].node->Equal(internal::Operator(MathOperator::MATH_MULTIPLY)));
    }

    {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("1*2-3");

        auto postOrder = internal::ParseFunctions::InOrderToPostOrder(tokens);

        ASSERT_TRUE(postOrder[0].node->Equal(Num(1)));
        ASSERT_TRUE(postOrder[1].node->Equal(Num(2)));
        ASSERT_TRUE(postOrder[2].node->Equal(internal::Operator(MathOperator::MATH_MULTIPLY)));
        ASSERT_TRUE(postOrder[3].node->Equal(Num(3)));
        ASSERT_TRUE(postOrder[4].node->Equal(internal::Operator(MathOperator::MATH_SUB)));
    }

    {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("2^3^4");

        auto postOrder = internal::ParseFunctions::InOrderToPostOrder(tokens);

        ASSERT_TRUE(postOrder[0].node->Equal(Num(2)));
        ASSERT_TRUE(postOrder[1].node->Equal(Num(3)));
        ASSERT_TRUE(postOrder[2].node->Equal(Num(4)));
        ASSERT_TRUE(postOrder[3].node->Equal(internal::Operator(MathOperator::MATH_POWER)));
        ASSERT_TRUE(postOrder[4].node->Equal(internal::Operator(MathOperator::MATH_POWER)));
    }
}
TEST(Parse, PostOrderError) {
    MemoryLeakDetection mld;

    {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("1*2-3)");

        try {
            auto postOrder = internal::ParseFunctions::InOrderToPostOrder(tokens);
            FAIL();
        } catch (const ParseError &err) {
            cout << err.what() << endl;
        }
    }

    {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("(1*2-3");

        try {
            auto postOrder = internal::ParseFunctions::InOrderToPostOrder(tokens);
            FAIL();
        } catch (const ParseError &err) {
            cout << err.what() << endl;
        }
    }
}
TEST(Parse, BuildTree) {
    MemoryLeakDetection mld;
    {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("1*(2-3)");
        auto postOrder = internal::ParseFunctions::InOrderToPostOrder(tokens);
        auto node = internal::ParseFunctions::BuildExpressionTree(postOrder);
        ASSERT_EQ(node->ToString(), "1*(2-3)");
        node->CheckParent();
    }

    {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("1*2-3");
        auto postOrder = internal::ParseFunctions::InOrderToPostOrder(tokens);
        auto node = internal::ParseFunctions::BuildExpressionTree(postOrder);
        ASSERT_EQ(node->ToString(), "1*2-3");
        node->CheckParent();
    }
}
TEST(Parse, Mix) {
    MemoryLeakDetection mld;

    {
        deque<internal::Token> tokens =
            internal::ParseFunctions::ParseToTokens("a*cos(x1) + b*cos(x1-x2) + c*cos(x1-x2-x3)");
        auto postOrder = internal::ParseFunctions::InOrderToPostOrder(tokens);
        auto node = internal::ParseFunctions::BuildExpressionTree(postOrder);
        node->CheckParent();

        Node expected = Var("a") * cos(Var("x1")) + Var("b") * cos(Var("x1") - Var("x2")) +
                        Var("c") * cos(Var("x1") - Var("x2") - Var("x3"));
        ASSERT_TRUE(node->Equal(expected));
    }

    try {
        deque<internal::Token> tokens = internal::ParseFunctions::ParseToTokens("x(1)*cos(2)");
        auto postOrder = internal::ParseFunctions::InOrderToPostOrder(tokens);
        auto node = internal::ParseFunctions::BuildExpressionTree(postOrder);
        FAIL();
    } catch (const ParseError &err) {
        cout << err.what() << endl;
    }

    try {
        deque<internal::Token> tokens =
            internal::ParseFunctions::ParseToTokens("x(1)*cos(x(2)) + x(2)*sin(x(1)) - 0.5");
        auto postOrder = internal::ParseFunctions::InOrderToPostOrder(tokens);
        auto node = internal::ParseFunctions::BuildExpressionTree(postOrder);
        FAIL();
    } catch (const ParseError &err) {
        cout << err.what() << endl;
    }
}

TEST(Power, Base) {
    MemoryLeakDetection mld;

    Node n = Num(2) ^ Num(3);
    ASSERT_EQ(n->ToString(), "2^3");
    ASSERT_DOUBLE_EQ(n->Vpa(), 8);

    // CATION!
    Node n2 = Num(2) ^ Num(3) ^ Num(2);
    ASSERT_EQ(n2->ToString(), "(2^3)^2");
    ASSERT_DOUBLE_EQ(n2->Vpa(), 64);

    // CATION!
    Node n3 = Num(2) ^ (Num(3) ^ Num(2));
    ASSERT_EQ(n3->ToString(), "2^(3^2)");
    ASSERT_DOUBLE_EQ(n3->Vpa(), 512);
}

TEST(Node, Random) {
    MemoryLeakDetection mld;

    int maxCount = 10;

    auto seed = static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    cout << "seed = " << seed << endl;
    std::default_random_engine eng(seed);

    std::uniform_int_distribution<int> unifCount(1, maxCount);
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
TEST(Clone, DoNotStackOverFlow) {
    MemoryLeakDetection mld;

    // 构造一个随机的长表达式
    auto pr = CreateRandomExpresionTree(10000);
    Node &node = pr.first;

    // clone，不应爆栈
    Node n2 = Clone(node);

    ASSERT_TRUE(node->Equal(n2));
}
TEST(Vpa, DoNotStackOverFlow) {
    MemoryLeakDetection mld;

    // 构造一个随机的长表达式
    auto pr = CreateRandomExpresionTree(10000);
    Node &node = pr.first;
    double v = pr.second;

    double result = node->Vpa();

    cout << "\t result = " << result << endl;
    cout << "\t expected = " << v << endl;
    ASSERT_DOUBLE_EQ(result, v);
}
TEST(ToString, DoNotStackOverFlow) {
    MemoryLeakDetection mld;

    // 构造一个随机的长表达式
    auto pr = CreateRandomExpresionTree(10000);
    Node &node = pr.first;

    // 输出表达式字符串，不应爆栈
    std::string s = node->ToString();
}

TEST(Simplify, Base) {
    MemoryLeakDetection mld;

    Node n = sin(Num(0));
    n->Simplify();

    ASSERT_EQ(n->ToString(), "0");

    Node n2 = Num(1) + Num(2) * Num(3);
    n2->Simplify();

    ASSERT_EQ(n2->ToString(), "7");

    ASSERT_TRUE(n2->Equal(Num(7)));
}
TEST(Simplify, Add) {
    MemoryLeakDetection mld;

    {
        Node n = Var("x") + Num(0);
        n->Simplify();
        ASSERT_EQ(n->ToString(), "x");
        n->CheckParent();
    }

    {
        Node n = Num(0) + Var("x");
        n->Simplify();
        ASSERT_EQ(n->ToString(), "x");
        n->CheckParent();
    }
}
TEST(Simplify, Multiply) {
    MemoryLeakDetection mld;

    {
        Node n = Var("x") * Num(1) * Var("y") * Var("z");
        n->Simplify();
        ASSERT_EQ(n->ToString(), "x*y*z");
        n->CheckParent();
    }

    {
        Node n = cos(Var("x")) * Num(1);
        n->Simplify();
        ASSERT_EQ(n->ToString(), "cos(x)");
        n->CheckParent();
    }

    {
        Node n = Num(1) * Var("x") * Num(0) + Num(0) * Var("y");
        n->Simplify();
        ASSERT_EQ(n->ToString(), "0");
        n->CheckParent();
    }
}
TEST(Simplify, DoNotStackOverFlow) {
    MemoryLeakDetection mld;

    // 构造一个随机的长表达式
    auto pr = CreateRandomExpresionTree(100000);
    Node &node = pr.first;

    node->Simplify();
}

TEST(SolveBase, FindAlphaByArmijo) {
    MemoryLeakDetection mld;

    GetConfig().epsilon = 1e-6;

    auto g = [](const Vec &x) -> Vec {
        return {pow(x[0] - 4, 4) + pow(x[1] - 3, 2) + 4 * pow(x[2] + 5, 4)};
    };

    auto dg = [](const Vec &x) -> Vec {
        return {4 * pow(x[0] - 4, 3), 2 * (x[1] - 3), 16 * pow(x[2] + 5, 3)};
    };

    Vec x{4, 2, -1};
    Vec d = -Vec{0, -2, 1024};
    double alpha = Armijo(x, d, g, dg);
    cout << alpha << endl;

    // FIXME: not match got results
    // double expected = 0.003866;
}
TEST(SolveBase, FindAlpha) {

    auto g = [](const Vec &x) -> Vec {
        return {pow(x[0] - 4, 4), pow(x[1] - 3, 2), 4 * pow(x[2] + 5, 4)};
    };

    Vec x{4, 2, -1};
    Vec d = -Vec{0, -2, 1024};
    double alpha = FindAlpha(x, d, g);
    cout << alpha << endl;

    // FIXME: not match got results
    // double expected = 0.003866;
}
TEST(SolveBase, Base) {
    // the example of this test is from: https://zhuanlan.zhihu.com/p/136889381

    MemoryLeakDetection mld;

    std::setlocale(LC_ALL, ".UTF8");

    /*
        以一个平面三轴机器人为例，运动学方程为
            a = 0.425;  b = 0.39243;  c=0.109;
            y = [   a*cos(x(1)) + b*cos(x(1)-x(2)) + c*cos(x(1)-x(2)-x(3)),
                    a*sin(x(1)) + b*sin(x(1)-x(2)) + c*sin(x(1)-x(2)-x(3)),
                    x(1)-x(2)-x(3)    ];
    */
    Node f1 = Var("a") * cos(Var("x1")) + Var("b") * cos(Var("x1") - Var("x2")) +
              Var("c") * cos(Var("x1") - Var("x2") - Var("x3"));
    Node f2 = Var("a") * sin(Var("x1")) + Var("b") * sin(Var("x1") - Var("x2")) +
              Var("c") * sin(Var("x1") - Var("x2") - Var("x3"));
    Node f3 = Var("x1") - Var("x2") - Var("x3");

    SymVec f{Clone(f1), Clone(f2), Clone(f3)};

    // 目标位置为：[0.5 0.4 0]
    SymVec b{Num(0.5), Num(0.4), Num(0)};
    SymVec equations = f - b;
    equations.Subs(VarsTable{{"a", 0.425}, {"b", 0.39243}, {"c", 0.109}});

    // 初值表
    VarsTable varsTable{{"x1", 1}, {"x2", 1}, {"x3", 1}};

    // 期望值
    VarsTable expected{{"x1", 1.5722855035930956}, {"x2", 1.6360330989069252}, {"x3", -0.0637475947386077}};

    // Newton-Raphson方法
    {
        VarsTable got = SolveByNewtonRaphson(varsTable, equations);
        cout << got << endl;

        ASSERT_EQ(got, expected);
    }

    // LM方法
    {
        VarsTable got = SolveByLM(varsTable, equations);
        cout << got << endl;

        ASSERT_EQ(got, expected);
    }
}

TEST(Solve, Base) {
    // the example of this test is from: https://zhuanlan.zhihu.com/p/136889381

    MemoryLeakDetection mld;

    std::setlocale(LC_ALL, ".UTF8");

    /*
        以一个平面三轴机器人为例，运动学方程为
            a = 0.425;  b = 0.39243;  c=0.109;
            y = [   a*cos(x(1)) + b*cos(x(1)-x(2)) + c*cos(x(1)-x(2)-x(3)),
                    a*sin(x(1)) + b*sin(x(1)-x(2)) + c*sin(x(1)-x(2)-x(3)),
                    x(1)-x(2)-x(3)    ];
    */
    Node f1 = Parse("a*cos(x1) + b*cos(x1-x2) + c*cos(x1-x2-x3)");
    Node f2 = Parse("a*sin(x1) + b*sin(x1-x2) + c*sin(x1-x2-x3)");
    Node f3 = Parse("x1-x2-x3");

    SymVec f{Move(f1), Move(f2), Move(f3)};

    // 目标位置为：[0.5 0.4 0]
    SymVec b{Num(0.5), Num(0.4), Num(0)};
    SymVec equations = f - b;
    equations.Subs(VarsTable{{"a", 0.425}, {"b", 0.39243}, {"c", 0.109}});

    // 期望值
    VarsTable expected{{"x1", 1.5722855035930956}, {"x2", 1.6360330989069252}, {"x3", -0.0637475947386077}};

    // Newton-Raphson方法
    {
        GetConfig().nonlinearMethod = NonlinearMethod::NEWTON_RAPHSON;

        VarsTable got = Solve(equations);
        cout << got << endl;

        ASSERT_EQ(got, expected);
    }

    // LM方法
    {
        GetConfig().nonlinearMethod = NonlinearMethod::LM;

        VarsTable got = Solve(equations);
        cout << got << endl;

        ASSERT_EQ(got, expected);
    }
}
TEST(Solve, Case1) {
    MemoryLeakDetection mld;

    std::setlocale(LC_ALL, ".UTF8");

    /*

    Matlab code:
    from: https://ww2.mathworks.cn/help/optim/ug/fsolve.html

    root2d.m:
        function F = root2d(x)
            F(1) = exp(-exp(-(x(1)+x(2)))) - x(2)*(1+x(1)^2);
            F(2) = x(1)*cos(x(2)) + x(2)*sin(x(1)) - 0.5;
        end

    root2d_solve.m:
        format long
        fun = @root2d;
        x0 = [0,0];
        x = fsolve(fun,x0)

    result:
        x =

            0.353246561920553   0.606082026502285


     */

    // 构造方程组
    SymVec f{{Parse("exp(-exp(-(x1 + x2))) - x2 * (1 + x1 ^ 2)"), Parse("x1 * cos(x2) + x2 * sin(x1) - 0.5")}};

    // 设置初值为0.0
    GetConfig().initialValue = 0.0;

    // 求解，结果保存到ans
    VarsTable ans = Solve(f);

    // 打印出ans
    cout << ans << endl;

    // 单独获取变量的值
    cout << "x1 = " << ans["x1"] << endl;
    cout << "x2 = " << ans["x2"] << endl;

    ASSERT_EQ(ans, VarsTable({{"x1", 0.353246561920553}, {"x2", 0.606082026502285}}));
}

TEST(Subs, Base) {
    MemoryLeakDetection mld;

    Node n = Var("x");
    ASSERT_EQ(Subs(n, "x", Var("y"))->ToString(), "y");

    ASSERT_EQ(Subs(n, "x", Num(100))->ToString(), "100");

    ASSERT_DOUBLE_EQ(Subs(std::move(n), "x", Num(99))->Vpa(), 99.0);
    ASSERT_EQ(n, nullptr);
}
TEST(Subs, Combine) {
    MemoryLeakDetection mld;

    {
        // x*y+sin(x)
        Node n = Var("x") * Var("y") + sin(Var("x"));

        n = Subs(std::move(n), "x", Var("x") + Num(1));

        ASSERT_EQ(n->ToString(), "(x+1)*y+sin(x+1)");
    }

    {
        // r*sin(x+y)
        Node n = Var("r") * sin(Var("x") + Var("y"));

        // -> 100*sin(360deg+30deg) == 50
        n = Subs(std::move(n), "x", Num(radians(360.0)));
        n = Subs(std::move(n), "y", Num(radians(30.0)));
        n = Subs(std::move(n), "r", Num(100));

        ASSERT_DOUBLE_EQ(n->Vpa(), 50.0);
    }
}
TEST(Subs, Multiple) {
    MemoryLeakDetection mld;

    {
        // x*y+sin(x)
        Node n = Var("x") * Var("y") + sin(Var("x"));

        // 交换x y
        n = Subs(std::move(n), {"x", "y"}, {Var("y"), Var("x")});
        ASSERT_EQ(n->ToString(), "y*x+sin(y)");

        // x -> cos(y)
        n = Subs(std::move(n), {"x"}, {cos(Var("y"))});
        ASSERT_EQ(n->ToString(), "y*cos(y)+sin(y)");
    }
}

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

TEST(ToString, Base) {
    MemoryLeakDetection mld;

    ASSERT_EQ(ToString(0.0), "0");
    ASSERT_EQ(ToString(1.0), "1");
    ASSERT_EQ(ToString(0.1), "0.1");
    ASSERT_EQ(ToString(0.12), "0.12");
    ASSERT_EQ(ToString(0.123456789123450), "0.12345678912345");
    ASSERT_EQ(ToString(1234567890.0), "1234567890");

    // 15位
    ASSERT_EQ(ToString(123456789012345), "123456789012345");
    ASSERT_EQ(ToString(-123456789012345), "-123456789012345");

    // 16位
    ASSERT_EQ(ToString(1234567890123456), "1234567890123456");
    ASSERT_EQ(ToString(-1234567890123456), "-1234567890123456");

    ASSERT_EQ(ToString(1.0e0), "1");
    ASSERT_EQ(ToString(1e0), "1");
    ASSERT_EQ(ToString(1e1), "10");
    ASSERT_EQ(ToString(1e15), "1000000000000000");
    ASSERT_EQ(ToString(1e16), "1e+16");
    ASSERT_EQ(ToString(1.0e16), "1e+16");
    ASSERT_EQ(ToString(1e-16), "9.9999999999999998e-17");
    ASSERT_EQ(ToString(1.0e-16), "9.9999999999999998e-17");

    ASSERT_EQ(ToString(std::numeric_limits<double>::min()), "2.2250738585072014e-308");
    ASSERT_EQ(ToString(std::numeric_limits<double>::max()), "1.7976931348623157e+308");
    ASSERT_EQ(ToString(std::numeric_limits<double>::denorm_min()), "4.9406564584124654e-324");
    ASSERT_EQ(ToString(std::numeric_limits<double>::lowest()), "-1.7976931348623157e+308");
}

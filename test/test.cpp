#include "node.h"

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

TEST(Node, Random) {
    MemoryLeakDetection mld;

    int maxCount = 100;

    default_random_engine eng(
        static_cast<unsigned int>(chrono::high_resolution_clock::now().time_since_epoch().count()));

    vector<MathOperator> ops{MathOperator::MATH_ADD, MathOperator::MATH_SUB, MathOperator::MATH_MULTIPLY,
                             MathOperator::MATH_DIVIDE};
    uniform_int_distribution<int> unifCount(1, maxCount);
    uniform_int_distribution<int> unifOp(0, ops.size() - 1);
    uniform_real_distribution<double> unifNum(-100.0, 100.0);
     for (int i=0; i < 100; ++i)
    {
        double v = 1.0;
        auto node = Num(1.0);

        int count = unifCount(eng);
        for (int j = 0; j < count; ++j)
        {
            double num = unifNum(eng);
            auto op = ops[unifOp(eng)];
            switch (op) {
            case MathOperator::MATH_ADD:
                v += num;
                node += Num(num);
                break;
            case MathOperator::MATH_SUB:
                v -= num;
                node -= Num(num);
                break;
            case MathOperator::MATH_MULTIPLY:
                v *= num;
                node *= Num(num);
                break;
            case MathOperator::MATH_DIVIDE:
                v /= num;
                node /= Num(num);
                break;
            default:
                assert(0);
            }
        }

        cout << node->ToString() << endl;
        double result = node->Vpa();
        cout << "\t result = " << result << endl;
        cout << "\t expected = " << v << endl;
        ASSERT_DOUBLE_EQ(result, v);
    }
}

TEST(Vec, Base)
{
}

/*
y = [   a*cos(x(1)) + b*cos(x(1)-x(2)) + c*cos(x(1)-x(2)-x(3)),
        a*sin(x(1)) + b*sin(x(1)-x(2)) + c*sin(x(1)-x(2)-x(3)),
        x(1)-x(2)-x(3)    ];

作者：无忌不悔
链接：https://zhuanlan.zhihu.com/p/136889381
来源：知乎
著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。
*/
// int main() {
//
//    // auto a = Var("a");
//
//    // auto expr = Var("a") * Func("cos");
//
//    //cout << "dsjfk" << endl;
//    return 0;
//}
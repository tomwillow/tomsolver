#include <tomsolver/error_type.h>
#include <tomsolver/functions.h>
#include <tomsolver/math_operator.h>
#include <tomsolver/node.h>

#include "memory_leak_detection.h"

#include <gtest/gtest.h>

#include <random>

using namespace tomsolver;

using std::cout;
using std::endl;

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

TEST(Node, Op) {
    MemoryLeakDetection mld;

    ASSERT_ANY_THROW(Op(MathOperator::MATH_NULL));

    ASSERT_NO_THROW(Op(MathOperator::MATH_ADD));
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
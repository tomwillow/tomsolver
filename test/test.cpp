#include "node.h"

#include "memory_leak_detection.h"

#include <gtest/gtest.h>

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
#include "node.h"

#include "memory_leak_detection.h"

#include <gtest/gtest.h>

/*
y = [   a*cos(x(1)) + b*cos(x(1)-x(2)) + c*cos(x(1)-x(2)-x(3)),
        a*sin(x(1)) + b*sin(x(1)-x(2)) + c*sin(x(1)-x(2)-x(3)),
        x(1)-x(2)-x(3)    ];

作者：无忌不悔
链接：https://zhuanlan.zhihu.com/p/136889381
来源：知乎
著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。
*/
//int main() {
//
//    // auto a = Var("a");
//
//    // auto expr = Var("a") * Func("cos");
//
//    //cout << "dsjfk" << endl;
//    return 0;
//}

using namespace std;
using namespace tomsolver;

TEST(Node, Basic)
{
    MemoryLeakDetection mld;

    auto n = Num(10);

    cout << n << endl;

    auto n2 = Num(1) + Num(2);

    cout << n2 << endl;
    //auto n3 = n + n2;

    return;
}
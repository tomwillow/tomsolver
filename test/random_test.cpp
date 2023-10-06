#include "node.h"

#include "memory_leak_detection.h"
#include "helper.h"

#include <gtest/gtest.h>

#include <random>

using namespace std;
using namespace tomsolver;

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
    string s = node->ToString();
}
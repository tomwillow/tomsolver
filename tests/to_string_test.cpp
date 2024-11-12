#include <tomsolver/config.h>

#include "memory_leak_detection.h"

#include <gtest/gtest.h>

#ifdef WIN32
#undef max
#undef min
#endif

using namespace tomsolver;

using std::cout;
using std::endl;

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
# tomsolver

![workflow](https://github.com/tomwillow/tomsolver/actions/workflows/cmake-multi-platform.yml/badge.svg)

[中文](https://github.com/tomwillow/tomsolver) [English](https://github.com/tomwillow/tomsolver/blob/master/README-en.md)

C++14 极简非线性方程组求解器

> 让C++求解非线性方程组像Matlab fsolve一样简单

地址: https://github.com/tomwillow/tomsolver

**Contributors:**

* Tom Willow (https://github.com/tomwillow)
* lizho (https://github.com/lizho)

# 特点

* 简单！简单！会用fsolve就会用这个！
* 单头文件，直接include完事儿！

# 功能

* 非线性方程组求解（牛顿-拉夫森法、LM方法）
* 线性方程组求解（高斯-列主元迭代法、逆矩阵）
* 矩阵、向量运算（矩阵求逆、向量叉乘等）
* “伪”符号运算（对表达式求导、对符号矩阵求雅可比矩阵）

# 跨平台支持

测试环境：

* Linux: ubuntu 22.04 x86_64 gcc 11.3.0
* Windows: windows10 x64 Visual Studio 2019

Github Actions自动测试：

* Linux-latest gcc Debug&Release
* Linux-latest clang Debug&Release
* Windows-latest msvc Debug&Release

# 例子

```C++
#include <tomsolver/tomsolver.hpp>

using namespace tomsolver;

int main() {
    /*
    Translate from Matlab code:

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
    SymVec f = {
        "exp(-exp(-(x1 + x2))) - x2 * (1 + x1 ^ 2)"_f,
        "x1 * cos(x2) + x2 * sin(x1) - 0.5"_f,
    };

    // 设置初值为0.0
    Config::Get().initialValue = 0.0;

    // 求解，结果保存到ans
    VarsTable ans = Solve(f);

    // 打印出ans
    std::cout << ans << std::endl;

    // 单独获取变量的值
    std::cout << "x1 = " << ans["x1"] << std::endl;
    std::cout << "x2 = " << ans["x2"] << std::endl;

    return 0;
}
```

# 用法

## 1. header-only用法

仅需要包含一个单头文件即可：
`single/include/tomsolver/tomsolver.hpp`

## 2. 二进制库+头文件用法

```bash
$ git clone https://github.com/tomwillow/tomsolver
$ mkdir build
$ cd build
$ cmake ../tomsolver
$ cmake --build . --target INSTALL
```

然后添加include目录，并链接到库文件。

# 目录结构

* **src**: 源文件
* **tests**: 单元测试
* **single/include**: header-only的tomsolver.hpp所在的文件夹
* **single/test**: 所有单元测试整合为一个.cpp文件，用于测试tomsolver.hpp是否正确
* **scripts**: 用于生成single下面的单文件头文件和单文件测试

### 例子

* **examples/solve**: 解非线性方程的例子，演示基本用法和怎么设置统一的初值
* **examples/solve2**: 解非线性方程的例子，演示怎么切换解法和怎么替换方程中的已知量
* **examples/diff_machine**: 求导器，输入一行表达式，输出这个表达式的求导结果

# 开发计划

* 增加doxygen注释+教程文档（CN+EN）
* 增加benchmark测速
* 增加使用Eigen库作为内置矩阵库的可选项
* 对标Matlab fsolve，增加更多非线性方程组解法
* 在Solve函数中增加可选的Config参数，可以使用非全局的Config进行求解
  （类似于Matlab fsolve的options）
* 增加对二元/多元函数的支持，例如pow(x, y)
* 现在的Simplify函数还很朴素，把Simplify修改得更好
* 增加LaTeX格式的公式输出

# Thanks

https://github.com/taehwan642

# 微信交流群

如果有问题想要交流或者想参与开发，或者想与作者联系，欢迎加微信tomwillow。备注tomsolver按照指引进群。

如果您觉得此项目不错，请赏颗星吧！

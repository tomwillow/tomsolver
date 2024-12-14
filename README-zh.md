# tomsolver

![workflow](https://github.com/tomwillow/tomsolver/actions/workflows/cmake-multi-platform.yml/badge.svg)

[中文](https://github.com/tomwillow/tomsolver/blob/master/README-zh.md) [English](https://github.com/tomwillow/tomsolver)

C++14 极简非线性方程组求解器

> 让 C++求解非线性方程组像 Matlab fsolve 一样简单

地址: https://github.com/tomwillow/tomsolver

**Contributors:**

- Tom Willow (https://github.com/tomwillow)
- lizho (https://github.com/lizho)

# 特点

- 简单！简单！会用 fsolve 就会用这个！
- 单头文件，直接 include 完事儿！

# 功能

- 非线性方程组求解（牛顿-拉夫森法、LM 方法）
- 线性方程组求解（高斯-列主元迭代法、逆矩阵）
- 矩阵、向量运算（矩阵求逆、向量叉乘等）
- “伪”符号运算（对表达式求导、对符号矩阵求雅可比矩阵）

# 跨平台支持

测试环境：

- Linux: ubuntu 22.04 x86_64 gcc 11.3.0
- Windows: windows10 x64 Visual Studio 2019

Github Actions 自动测试：

- Linux-latest gcc Debug&Release
- Linux-latest clang Debug&Release
- Windows-latest msvc Debug&Release

# 例子

```C++
#include <tomsolver/tomsolver.h>

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
    try {
        // 创建方程组
        SymVec f = {
            "exp(-exp(-(x1 + x2))) - x2 * (1 + x1 ^ 2)"_f,
            "x1 * cos(x2) + x2 * sin(x1) - 0.5"_f,
        };

        // 求解！
        VarsTable ans = Solve(f);

        // 打印结果
        std::cout << ans << std::endl;

        // 取得结果
        std::cout << "x1 = " << ans["x1"] << std::endl;
        std::cout << "x2 = " << ans["x2"] << std::endl;
    } catch (const std::runtime_error &err) {
        // 如果出错，捕获异常，并打印
        std::cerr << err.what() << std::endl;
        return -1;
    }

    return 0;
}
```

# 用法

## 1. header-only用法

仅需要包含一个单头文件即可：
`single/include/tomsolver/tomsolver.hpp`

## 2. 使用VCPKG

你可以使用[vcpkg](https://learn.microsoft.com/zh-cn/vcpkg/get_started/overview)进行统一的包管理。

* 清单模式（推荐）
  新建 `vcpkg.json`文件：

  ```
  {
    "dependencies": [
      "tomsolver"
    ]
  }
  ```

  然后在当前目录执行：
  `$ vcpkg install`
  详细教程：[https://learn.microsoft.com/zh-cn/vcpkg/get_started/get-started?pivots=shell-powershell]()
* 经典模式
  直接执行：
  `$ vcpkg install tomsolver`

## 3. 二进制库+头文件用法

```bash
$ git clone https://github.com/tomwillow/tomsolver
$ mkdir build
$ cd build
$ cmake ../tomsolver
$ cmake --build . --target INSTALL
```

然后添加 include 目录，并链接到库文件。

# 目录结构

- **src**: 源文件
- **tests**: 单元测试
- **single/include**: header-only 的 tomsolver.h 所在的文件夹
- **single/test**: 所有单元测试整合为一个.cpp 文件，用于测试 tomsolver.h 是否正确
- **scripts**: 用于生成 single 下面的单文件头文件和单文件测试

### 例子

- **examples/solve**: 解非线性方程的例子，演示基本用法和怎么设置统一的初值
- **examples/solve2**: 解非线性方程的例子，演示怎么切换解法和怎么替换方程中的已知量
- **examples/diff_machine**: 求导器，输入一行表达式，输出这个表达式的求导结果

# 开发计划

- 增加 doxygen 注释+教程文档（CN+EN）
- 增加 benchmark 测速
- 增加使用 Eigen 库作为内置矩阵库的可选项
- 对标 Matlab fsolve，增加更多非线性方程组解法
- 在 Solve 函数中增加可选的 Config 参数，可以使用非全局的 Config 进行求解
  （类似于 Matlab fsolve 的 options）
- 增加对二元/多元函数的支持，例如 pow(x, y)
- 现在的 Simplify 函数还很朴素，把 Simplify 修改得更好
- 增加 LaTeX 格式的公式输出

# Thanks

https://github.com/taehwan642

# 微信交流群

如果有问题想要交流或者想参与开发，或者想与作者联系，欢迎加微信 tomwillow。备注 tomsolver 按照指引进群。

如果您觉得此项目不错，请赏颗星吧！

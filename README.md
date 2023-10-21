# tomsolver

![workflow](https://github.com/tomwillow/tomsolver/actions/workflows/cmake-multi-platform.yml/badge.svg)

C++14 极简非线性方程组求解器

"让C++求解非线性方程组像Matlab fsolve一样简单"

地址: https://github.com/tomwillow/tomsolver

作者: Tom Willow

# 特点

* 非线性方程组求解（牛顿-拉夫森法、LM方法）
* 线性方程组求解（高斯-列主元迭代法、逆矩阵）
* 矩阵、向量运算（矩阵求逆、向量叉乘等）
* “伪”符号运算（对表达式求导、对符号矩阵求雅可比矩阵）

# 跨平台支持

支持的操作系统: Linux, Windows
支持的编译器: g++, clang, msvc

代码已经在以上环境通过Github Actions测试。

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

    from: https://ww2.mathworks.cn/help/optim/ug/fsolve.html
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

    return 0;
}
```

# 用法

## 1. head-only用法

仅需要一个单头文件即可：
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

# Thanks

https://github.com/taehwan642

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
    SymVec f{{Parse("exp(-exp(-(x1 + x2))) - x2 * (1 + x1 ^ 2)"), Parse("x1 * cos(x2) + x2 * sin(x1) - 0.5")}};

    // 设置初值为0.0
    Config::get().initialValue = 0.0;

    // 求解，结果保存到ans
    VarsTable ans = Solve(f);

    // 打印出ans
    std::cout << ans << std::endl;

    // 单独获取变量的值
    std::cout << "x1 = " << ans["x1"] << std::endl;
    std::cout << "x2 = " << ans["x2"] << std::endl;

    return 0;
}
#include <tomsolver/tomsolver.hpp>

using namespace tomsolver;

int main() {
    std::setlocale(LC_ALL, ".UTF8");

    // 使用LM方法
    Config::Get().nonlinearMethod = NonlinearMethod::LM;

    // 定义方程，实质上是符号向量，求解时就相当于求解符号向量f=0
    SymVec f{
        Parse("a/(b^2)-c/(d^2)"),
        Parse("129.56108*b-(a/(b^2)+1/a-2*b/(a^2))"),
        Parse("129.56108*d-(d/(c^2)-c/(d^2)-1/a)"),
        Parse("5*e-7-(2/3*pi*a^2*b+((sqrt(3)*c^2)/(3*sqrt(c^2/3+d^2))+a-c)^2*pi*d^2/(c^2/3+d^2))"),
    };

    // 把pi, e替换为pi, e的数值
    f.Subs(VarsTable{{"pi", PI}, {"e", std::exp(1.0)}});

    cout << f << endl;

    // 求解
    auto ans = Solve(f);

    cout << ans << endl;

    return 0;
}